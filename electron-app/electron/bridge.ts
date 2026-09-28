// ---------------------------------------------------------------------------
// DnsCoreBridge: spawns the C++ dns-core sidecar and speaks NDJSON JSON-RPC
// over its stdio pipes. The sidecar is built from the backend sources
// (core/, models/, platform/) — this layer is transport only and contains
// no DNS/business logic of its own.
// ---------------------------------------------------------------------------

import { spawn, ChildProcess } from "node:child_process";
import { EventEmitter } from "node:events";
import { existsSync } from "node:fs";
import * as path from "node:path";

export interface CoreNotification {
  text: string;
  kind: "info" | "success" | "warning" | "error";
}

// Generous for UAC elevation (the C++ side waits up to 30 s for approval).
const kDefaultTimeoutMs = 15_000;
const kMutatingTimeoutMs = 90_000;
const kMutatingMethods = new Set(["applyProfile", "resetDns", "benchmark"]);
const kMaxRestarts = 5;

interface PendingCall {
  resolve: (value: Record<string, unknown>) => void;
  reject: (err: Error) => void;
  timer: NodeJS.Timeout;
}

export function resolveDnsCorePath(appRoot: string, isPackaged: boolean): string {
  if (isPackaged) {
    return path.join(process.resourcesPath, "bin", "dns-core.exe");
  }
  // Dev: <repo>/electron-app -> <repo>/build/<preset>/dns-core.exe
  const repoRoot = path.resolve(appRoot, "..");
  const candidates = [
    path.join(repoRoot, "build", "ninja-release", "dns-core.exe"),
    path.join(repoRoot, "build", "vs2022-release", "Release", "dns-core.exe"),
    path.join(repoRoot, "build", "vs2026-release", "Release", "dns-core.exe"),
  ];
  for (const candidate of candidates) {
    if (existsSync(candidate)) return candidate;
  }
  return candidates[0];
}

export class DnsCoreBridge extends EventEmitter {
  private proc: ChildProcess | null = null;
  private nextId = 1;
  private pending = new Map<number, PendingCall>();
  private lineBuffer = "";
  private ready: Promise<void> | null = null;
  private readyResolve: (() => void) | null = null;
  private restarts = 0;
  private stopped = false;
  private writeChain: Promise<void> = Promise.resolve();

  constructor(
    private readonly binaryPath: string,
    private readonly logger: (msg: string) => void = () => {},
  ) {
    super();
  }

  start(): void {
    this.stopped = false;
    this.spawn();
  }

  stop(): void {
    this.stopped = true;
    for (const [, call] of this.pending) {
      clearTimeout(call.timer);
      call.reject(new Error("dns-core is shutting down"));
    }
    this.pending.clear();
    this.proc?.kill();
    this.proc = null;
  }

  isRunning(): boolean {
    return this.proc !== null && this.proc.exitCode === null;
  }

  private spawn(): void {
    this.log(`spawning dns-core: ${this.binaryPath}`);
    this.ready = new Promise((resolve) => {
      this.readyResolve = resolve;
    });
    const proc = spawn(this.binaryPath, [], {
      stdio: ["pipe", "pipe", "pipe"],
      windowsHide: true,
    });
    this.proc = proc;
    this.lineBuffer = "";

    proc.stdout?.on("data", (chunk: Buffer) => this.onStdout(chunk));
    proc.stderr?.on("data", (chunk: Buffer) => {
      this.log(`dns-core stderr: ${chunk.toString("utf8").trim()}`);
    });
    proc.on("error", (err) => {
      this.log(`dns-core spawn error: ${err.message}`);
      this.emit("down", err);
      this.failAllPending(err);
      this.maybeRestart();
    });
    proc.on("exit", (code, signal) => {
      this.log(`dns-core exited code=${code} signal=${signal}`);
      this.proc = null;
      this.failAllPending(new Error(`dns-core exited unexpectedly (code ${code})`));
      this.emit("down", new Error(`exit code ${code}`));
      this.maybeRestart();
    });
  }

  private maybeRestart(): void {
    if (this.stopped || this.restarts >= kMaxRestarts) {
      if (!this.stopped) this.emit("fatal");
      return;
    }
    this.restarts += 1;
    this.log(`restarting dns-core (attempt ${this.restarts}/${kMaxRestarts})`);
    setTimeout(() => {
      if (!this.stopped) this.spawn();
    }, 1000 * this.restarts);
  }

  private failAllPending(err: Error): void {
    for (const [, call] of this.pending) {
      clearTimeout(call.timer);
      call.reject(err);
    }
    this.pending.clear();
  }

  private onStdout(chunk: Buffer): void {
    this.lineBuffer += chunk.toString("utf8");
    let index: number;
    while ((index = this.lineBuffer.indexOf("\n")) >= 0) {
      const line = this.lineBuffer.slice(0, index).trim();
      this.lineBuffer = this.lineBuffer.slice(index + 1);
      if (line.length === 0) continue;
      this.onLine(line);
    }
  }

  private onLine(line: string): void {
    let msg: {
      id?: number;
      ok?: boolean;
      result?: Record<string, unknown>;
      error?: { message?: string; code?: number };
      event?: string;
      payload?: CoreNotification & Record<string, unknown>;
    };
    try {
      msg = JSON.parse(line);
    } catch {
      this.log(`dns-core sent invalid JSON: ${line.slice(0, 200)}`);
      return;
    }
    if (msg.event === "ready") {
      this.restarts = 0;
      this.readyResolve?.();
      this.emit("ready", msg.payload ?? {});
      return;
    }
    if (msg.event === "notification") {
      this.emit("notification", {
        text: String(msg.payload?.text ?? ""),
        kind: msg.payload?.kind ?? "info",
      } satisfies CoreNotification);
      return;
    }
    if (typeof msg.id === "number") {
      const call = this.pending.get(msg.id);
      if (!call) return;
      this.pending.delete(msg.id);
      clearTimeout(call.timer);
      if (msg.ok) {
        call.resolve(msg.result ?? {});
      } else {
        call.reject(
          new Error(msg.error?.message ?? "dns-core request failed"),
        );
      }
    }
  }

  async call(
    method: string,
    params: Record<string, unknown> = {},
  ): Promise<Record<string, unknown>> {
    await this.ready;
    if (!this.isRunning()) throw new Error("dns-core is not running");

    const id = this.nextId++;
    const timeoutMs = kMutatingMethods.has(method)
      ? kMutatingTimeoutMs
      : kDefaultTimeoutMs;
    const line = JSON.stringify({ id, method, params }) + "\n";

    // Serialize writes so piped stdin never interleaves frames.
    const ticket = this.writeChain.then(
      () =>
        new Promise<Record<string, unknown>>((resolve, reject) => {
          const timer = setTimeout(() => {
            this.pending.delete(id);
            reject(new Error(`dns-core request timed out (${method})`));
          }, timeoutMs);
          this.pending.set(id, { resolve, reject, timer });
          this.proc?.stdin?.write(line, (err) => {
            if (err) {
              this.pending.delete(id);
              clearTimeout(timer);
              reject(err);
            }
          });
        }),
    );
    this.writeChain = ticket.then(
      () => undefined,
      () => undefined,
    );
    return ticket;
  }

  private log(msg: string): void {
    try {
      this.logger(msg);
    } catch {
      /* logger must never break the bridge */
    }
  }
}

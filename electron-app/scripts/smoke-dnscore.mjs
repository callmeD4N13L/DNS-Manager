// Smoke test for the dns-core NDJSON protocol. Spawns the sidecar, runs a
// read-only sequence (getState, profiles.list, settings.get, invalid method,
// malformed JSON) and asserts the envelopes. Never mutates system DNS.
// Usage: node scripts/smoke-dnscore.mjs [path/to/dns-core.exe]

import { spawn } from "node:child_process";
import { existsSync } from "node:fs";
import * as path from "node:path";

const candidates = [
  process.argv[2],
  path.resolve(process.cwd(), "../build/ninja-release/dns-core.exe"),
  path.resolve(process.cwd(), "build/ninja-release/dns-core.exe"),
].filter(Boolean);

const binary = candidates.find((p) => p && existsSync(p));
if (!binary) {
  console.error(`dns-core.exe not found. Tried:\n  ${candidates.join("\n  ")}`);
  process.exit(2);
}

console.log(`smoke: using ${binary}`);
const child = spawn(binary, [], { stdio: ["pipe", "pipe", "inherit"], windowsHide: true });

let buffer = "";
const waiters = new Map();
let nextId = 1;
let readySeen = false;

child.stdout.on("data", (chunk) => {
  buffer += chunk.toString("utf8");
  let i;
  while ((i = buffer.indexOf("\n")) >= 0) {
    const line = buffer.slice(0, i).trim();
    buffer = buffer.slice(i + 1);
    if (!line) continue;
    let msg;
    try {
      msg = JSON.parse(line);
    } catch {
      console.error("smoke: invalid JSON from sidecar");
      process.exit(1);
    }
    if (msg.event === "ready") {
      readySeen = true;
      continue;
    }
    if (msg.event === "notification") continue; // async toast, ignore here
    const w = waiters.get(msg.id);
    if (w) {
      waiters.delete(msg.id);
      w(msg);
    }
  }
});

function call(method, params = {}) {
  const id = nextId++;
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      waiters.delete(id);
      reject(new Error(`timeout waiting for ${method}`));
    }, 20000);
    waiters.set(id, (msg) => {
      clearTimeout(timer);
      resolve(msg);
    });
    child.stdin.write(JSON.stringify({ id, method, params }) + "\n");
  });
}

function assert(cond, label) {
  if (!cond) {
    console.error(`smoke FAIL: ${label}`);
    child.kill();
    process.exit(1);
  }
  console.log(`smoke ok: ${label}`);
}

const results = [];
try {
  // Wait for the ready handshake (up to 20 s — first run enumerates adapters).
  const t0 = Date.now();
  while (!readySeen && Date.now() - t0 < 20000) await new Promise((r) => setTimeout(r, 100));

  const state = await call("getState");
  results.push(["getState ok", state.ok === true]);
  results.push(["adapters array", Array.isArray(state.result?.adapters)]);
  results.push(["profiles seeded >= 4", (state.result?.profiles ?? []).length >= 4]);
  results.push(["settings present", !!state.result?.settings?.themeMode]);
  results.push(["currentDns present", !!state.result?.currentDns]);
  console.log(
    `smoke: ${state.result.adapters.length} adapter(s), ${(state.result.profiles ?? []).length} profile(s), selected=${state.result.selectedAdapterId || "none"}`,
  );

  const list = await call("profiles.list", { search: "cloud" });
  results.push(["profiles.list search", list.ok === true && (list.result?.profiles ?? []).length >= 1]);

  const settings = await call("settings.get");
  results.push(["settings.get", settings.ok === true]);

  const bad = await call("nope.unknown");
  results.push(["unknown method rejected", bad.ok === false]);

  child.stdin.write("this is not json\n");
  const after = await call("getState");
  results.push(["recovers after malformed JSON", after.ok === true]);

  // Write paths (profile JSON + settings INI only — never touch system DNS).
  const badAdd = await call("profiles.add", { name: "   " });
  results.push(["invalid profile rejected", badAdd.ok === false]);

  const added = await call("profiles.add", {
    name: "smoke-test-profile",
    provider: "smoke",
    primaryIpv4: "1.1.1.1",
    secondaryIpv4: "1.0.0.1",
  });
  results.push(["profiles.add", added.ok === true && typeof added.result?.id === "string"]);
  const newId = added.result?.id;

  const listed = await call("profiles.list", { search: "smoke-test-profile" });
  results.push([
    "added profile listed",
    listed.ok === true && (listed.result?.profiles ?? []).some((p) => p.id === newId),
  ]);

  const fav = await call("profiles.toggleFavorite", { id: newId });
  results.push(["toggleFavorite", fav.ok === true]);

  const upd = await call("profiles.update", { id: newId, name: "smoke-test-renamed", primaryIpv4: "8.8.8.8" });
  results.push(["profiles.update", upd.ok === true]);

  const removed = await call("profiles.remove", { id: newId });
  results.push([
    "profiles.remove",
    removed.ok === true && !(removed.result?.profiles ?? []).some((p) => p.id === newId),
  ]);

  const setDark = await call("settings.update", { themeMode: "dark" });
  results.push(["settings.update", setDark.ok === true && setDark.result?.settings?.themeMode === "dark"]);
  const setBack = await call("settings.update", { themeMode: "system" });
  results.push(["settings.update revert", setBack.ok === true]);

  const badSettings = await call("settings.update", { themeMode: "neon" });
  // C++ ignores unknown theme values and returns current settings (still ok).
  results.push(["settings.update validation", badSettings.ok === true]);

  let failed = 0;
  for (const [label, cond] of results) {
    if (cond) console.log(`smoke ok: ${label}`);
    else {
      console.error(`smoke FAIL: ${label}`);
      failed++;
    }
  }
  assert(failed === 0, "all assertions passed");
} catch (err) {
  console.error(`smoke FAIL: ${err.message}`);
  child.kill();
  process.exit(1);
}
child.kill();
process.exit(0);

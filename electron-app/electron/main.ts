// ---------------------------------------------------------------------------
// Electron main process: owns the BrowserWindow, the system tray, the native
// file dialogs and the DnsCoreBridge subprocess. The renderer NEVER talks to
// dns-core directly — every channel below is an explicit, validated API.
// Security: contextIsolation=true, nodeIntegration=false, sandbox=true,
// no remote module, preload-only bridge, strict allowlist of channels.
// ---------------------------------------------------------------------------

import { app, BrowserWindow, Tray, Menu, nativeImage, dialog, ipcMain } from "electron";
import * as path from "node:path";
import { DnsCoreBridge, resolveDnsCorePath } from "./bridge";

let mainWindow: BrowserWindow | null = null;
let tray: Tray | null = null;
let isQuitting = false;
let minimizeToTray = true;

const bridge = new DnsCoreBridge(
  resolveDnsCorePath(app.getAppPath(), app.isPackaged),
  (msg) => console.log(`[dns-core] ${msg}`),
);

// --- strict input validation ----------------------------------------------
function isRecord(v: unknown): v is Record<string, unknown> {
  return typeof v === "object" && v !== null && !Array.isArray(v);
}

function asId(v: unknown): string | null {
  if (typeof v !== "string") return null;
  const s = v.trim();
  if (s.length === 0 || s.length > 128) return null;
  // UUID-ish / identifier characters only.
  if (!/^[A-Za-z0-9_.:@\-\\/ ]+$/.test(s)) return null;
  return s;
}

function asProfileFields(v: unknown): Record<string, unknown> | null {
  if (!isRecord(v)) return null;
  const out: Record<string, unknown> = {};
  const strKeys = [
    "name",
    "provider",
    "description",
    "primaryIpv4",
    "secondaryIpv4",
    "primaryIpv6",
    "secondaryIpv6",
  ];
  for (const key of strKeys) {
    if (v[key] === undefined) continue;
    if (typeof v[key] !== "string" || (v[key] as string).length > 256) return null;
    out[key] = (v[key] as string).trim();
  }
  if (typeof out.name === "string" && out.name.length === 0 && v["name"] !== undefined)
    return null;
  if (v["isFavorite"] !== undefined) {
    if (typeof v["isFavorite"] !== "boolean") return null;
    out["isFavorite"] = v["isFavorite"];
  }
  if (v["id"] !== undefined) {
    const id = asId(v["id"]);
    if (!id) return null;
    out["id"] = id;
  }
  return out;
}

function asSettingsPatch(v: unknown): Record<string, unknown> | null {
  if (!isRecord(v)) return null;
  const out: Record<string, unknown> = {};
  if (v["themeMode"] !== undefined) {
    if (v["themeMode"] !== "system" && v["themeMode"] !== "light" && v["themeMode"] !== "dark")
      return null;
    out["themeMode"] = v["themeMode"];
  }
  for (const key of [
    "startWithWindows",
    "startMinimized",
    "minimizeToTray",
    "flushCacheAfterApply",
    "confirmBeforeApply",
  ]) {
    if (v[key] !== undefined) {
      if (typeof v[key] !== "boolean") return null;
      out[key] = v[key];
    }
  }
  return out;
}

function asSearch(v: unknown): Record<string, unknown> {
  if (!isRecord(v)) return {};
  if (typeof v["search"] !== "string") return {};
  return { search: v["search"].slice(0, 128) };
}

// --- window ----------------------------------------------------------------
function createWindow(): void {
  mainWindow = new BrowserWindow({
    width: 1280,
    height: 800,
    minWidth: 960,
    minHeight: 620,
    title: "DNS Manager",
    backgroundColor: "#0a0f16",
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
    },
  });

  if (app.isPackaged || process.argv.includes("--smoke-test")) {
    mainWindow.loadFile(path.join(app.getAppPath(), "renderer", "dist", "index.html"));
  } else {
    mainWindow.loadURL("http://localhost:5173");
    // mainWindow.webContents.openDevTools({ mode: "detach" });
  }

  mainWindow.on("close", (event) => {
    if (!isQuitting && minimizeToTray && tray) {
      event.preventDefault();
      mainWindow?.hide();
    }
  });

  mainWindow.on("closed", () => {
    mainWindow = null;
  });
}

function ensureTray(): void {
  if (tray) return;
  const iconPath = app.isPackaged
    ? path.join(process.resourcesPath, "app.ico")
    : path.resolve(app.getAppPath(), "..", "resources", "win", "app.ico");
  let image = nativeImage.createFromPath(iconPath);
  if (image.isEmpty()) image = nativeImage.createEmpty();
  tray = new Tray(image);
  tray.setToolTip("DNS Manager");
  tray.setContextMenu(
    Menu.buildFromTemplate([
      {
        label: "Open DNS Manager",
        click: () => {
          mainWindow?.show();
          mainWindow?.focus();
        },
      },
      {
        label: "Flush DNS cache",
        click: () => {
          bridge.call("flushCache", {}).catch((e) => console.error(e));
        },
      },
      { type: "separator" },
      {
        label: "Quit",
        click: () => {
          isQuitting = true;
          app.quit();
        },
      },
    ]),
  );
  tray.on("double-click", () => {
    mainWindow?.show();
    mainWindow?.focus();
  });
}

// --- IPC allowlist (renderer -> main -> dns-core) ---------------------------
function registerIpc(): void {
  const forward = (
    channel: string,
    validate: (p: unknown) => Record<string, unknown> | null,
    method: string,
  ) => {
    ipcMain.handle(channel, async (_event, payload: unknown) => {
      const params = validate(payload);
      if (!params) throw new Error(`Invalid parameters for ${channel}`);
      try {
        return await bridge.call(method, params);
      } catch (err) {
        throw new Error(err instanceof Error ? err.message : String(err));
      }
    });
  };

  forward("dns:get-state", asSearch, "getState");
  forward("dns:refresh", () => ({}), "refresh");
  forward(
    "dns:select-adapter",
    (p) => {
      if (!isRecord(p)) return null;
      const id = asId(p["id"]);
      return id ? { id } : null;
    },
    "selectAdapter",
  );
  forward(
    "dns:apply-profile",
    (p) => {
      if (!isRecord(p)) return null;
      const id = asId(p["id"]);
      return id ? { id } : null;
    },
    "applyProfile",
  );
  forward("dns:reset", () => ({}), "resetDns");
  forward("dns:flush-cache", () => ({}), "flushCache");
  forward("dns:benchmark", () => ({}), "benchmark");

  forward("dns:profiles-list", asSearch, "profiles.list");
  forward("dns:profiles-add", asProfileFields, "profiles.add");
  forward(
    "dns:profiles-update",
    (p) => {
      const fields = asProfileFields(p);
      if (!fields || typeof fields["id"] !== "string") return null;
      return fields;
    },
    "profiles.update",
  );
  const byId = (p: unknown): Record<string, unknown> | null => {
    if (!isRecord(p)) return null;
    const id = asId(p["id"]);
    return id ? { id } : null;
  };
  forward("dns:profiles-remove", byId, "profiles.remove");
  forward(
    "dns:profiles-toggle-favorite",
    byId,
    "profiles.toggleFavorite",
  );
  forward("dns:profiles-reset", () => ({}), "profiles.reset");

  forward("dns:settings-get", () => ({}), "settings.get");
  forward("dns:settings-update", asSettingsPatch, "settings.update");
  forward("dns:settings-reset", () => ({}), "settings.reset");

  // Import/export paths NEVER come from renderer free-text: the main process
  // opens the trusted native file dialog and forwards the chosen path.
  ipcMain.handle("dns:profiles-import-dialog", async () => {
    if (!mainWindow) throw new Error("No window");
    const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
      title: "Import DNS profiles",
      filters: [{ name: "DNS profiles", extensions: ["json"] }],
      properties: ["openFile"],
    });
    if (canceled || filePaths.length === 0) return { canceled: true };
    return { ...(await bridge.call("profiles.import", { path: filePaths[0] })), canceled: false };
  });

  ipcMain.handle("dns:profiles-export-dialog", async () => {
    if (!mainWindow) throw new Error("No window");
    const { canceled, filePath } = await dialog.showSaveDialog(mainWindow, {
      title: "Export DNS profiles",
      defaultPath: "dns_profiles.json",
      filters: [{ name: "DNS profiles", extensions: ["json"] }],
    });
    if (canceled || !filePath) return { canceled: true };
    return { ...(await bridge.call("profiles.export", { path: filePath })), canceled: false };
  });

  ipcMain.handle("dns:window-hide", () => {
    mainWindow?.hide();
  });
  ipcMain.handle("dns:window-quit", () => {
    isQuitting = true;
    app.quit();
  });
}

// --- app lifecycle ----------------------------------------------------------
const gotLock = app.requestSingleInstanceLock();
if (!gotLock) {
  app.quit();
} else {
  app.on("second-instance", () => {
    mainWindow?.show();
    mainWindow?.focus();
  });

  app.whenReady().then(() => {
    registerIpc();
    bridge.on("notification", (n) => {
      mainWindow?.webContents.send("dns:notification", n);
    });
    bridge.on("ready", (meta) => {
      mainWindow?.webContents.send("dns:backend-ready", meta);
    });
    bridge.on("fatal", () => {
      mainWindow?.webContents.send("dns:backend-fatal", {
        text: "The DNS backend stopped unexpectedly.",
      });
    });
    bridge.start();
    // Keep minimizeToTray in sync (also readable without a window).
    bridge
      .call("settings.get", {})
      .then((r) => {
        const s = r["settings"] as Record<string, unknown> | undefined;
        if (s && typeof s["minimizeToTray"] === "boolean")
          minimizeToTray = s["minimizeToTray"] as boolean;
      })
      .catch(() => undefined);
    const startHidden =
      process.argv.includes("--minimized") || process.argv.includes("--hidden");
    createWindow();
    ensureTray();
    if (startHidden) mainWindow?.hide();

    // CI / verification hook: --smoke-test loads the built renderer, waits
    // for React to paint real backend data, reports, then exits.
    if (process.argv.includes("--smoke-test")) {
      const win = mainWindow;
      win?.webContents.on("did-finish-load", () => {
        const started = Date.now();
        const poll = async () => {
          try {
            const text = await win.webContents.executeJavaScript(
              "document.body.innerText.slice(0, 400)",
              true,
            );
            if (typeof text === "string" && /DNS Manager/.test(text) && /ms|DHCP|profiles/i.test(text)) {
              console.log(`SMOKE-OK renderer painted (${Date.now() - started}ms):`);
              console.log(String(text).slice(0, 300));
              bridge.stop();
              app.exit(0);
              return;
            }
          } catch {
            /* keep polling */
          }
          if (Date.now() - started > 45000) {
            console.log("SMOKE-FAIL renderer did not paint backend data in time");
            bridge.stop();
            app.exit(1);
            return;
          }
          setTimeout(poll, 750);
        };
        setTimeout(poll, 1500);
      });
    }
  });

  app.on("window-all-closed", () => {
    // Stay alive in the tray on Windows.
  });

  app.on("before-quit", () => {
    isQuitting = true;
    bridge.stop();
  });

  // Refresh the cached tray behavior whenever settings change.
  ipcMain.handle("dns:sync-tray-pref", (_event, payload: unknown) => {
    if (isRecord(payload) && typeof payload["minimizeToTray"] === "boolean")
      minimizeToTray = payload["minimizeToTray"];
  });
}

// ---------------------------------------------------------------------------
// Preload: the ONLY bridge between the renderer and the main process.
// contextIsolation=true, nodeIntegration=false, sandbox=true — the renderer
// gets a frozen, minimal, typed API and nothing else.
// ---------------------------------------------------------------------------

import { contextBridge, ipcRenderer } from "electron";

export interface BackendNotification {
  text: string;
  kind: "info" | "success" | "warning" | "error";
}

const api = {
  getState: (search = ""): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:get-state", { search }),
  refresh: (): Promise<Record<string, unknown>> => ipcRenderer.invoke("dns:refresh", {}),
  selectAdapter: (id: string): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:select-adapter", { id }),
  applyProfile: (id: string): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:apply-profile", { id }),
  resetDns: (): Promise<Record<string, unknown>> => ipcRenderer.invoke("dns:reset", {}),
  flushCache: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:flush-cache", {}),
  benchmark: (): Promise<Record<string, unknown>> => ipcRenderer.invoke("dns:benchmark", {}),

  listProfiles: (search = ""): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-list", { search }),
  addProfile: (fields: Record<string, unknown>): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-add", fields),
  updateProfile: (fields: Record<string, unknown>): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-update", fields),
  removeProfile: (id: string): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-remove", { id }),
  toggleFavorite: (id: string): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-toggle-favorite", { id }),
  resetConfiguration: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-reset", {}),
  importDialog: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-import-dialog"),
  exportDialog: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:profiles-export-dialog"),

  getSettings: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:settings-get", {}),
  updateSettings: (patch: Record<string, unknown>): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:settings-update", patch),
  resetSettings: (): Promise<Record<string, unknown>> =>
    ipcRenderer.invoke("dns:settings-reset", {}),
  syncTrayPref: (minimizeToTray: boolean): Promise<void> =>
    ipcRenderer.invoke("dns:sync-tray-pref", { minimizeToTray }),

  hideWindow: (): Promise<void> => ipcRenderer.invoke("dns:window-hide"),
  quitApp: (): Promise<void> => ipcRenderer.invoke("dns:window-quit"),

  // Custom frameless title-bar controls.
  minimizeWindow: (): Promise<void> => ipcRenderer.invoke("dns:window-min"),
  toggleMaximize: (): Promise<{ maximized: boolean }> =>
    ipcRenderer.invoke("dns:window-max-toggle"),
  isMaximized: (): Promise<{ maximized: boolean }> =>
    ipcRenderer.invoke("dns:window-is-maximized"),
  closeWindow: (): Promise<void> => ipcRenderer.invoke("dns:window-close"),
  onMaxChanged: (cb: (maximized: boolean) => void): (() => void) => {
    const listener = (_e: unknown, v: { maximized: boolean }) =>
      cb(!!v?.maximized);
    ipcRenderer.on("dns:window-max-changed", listener as (...a: unknown[]) => void);
    return () => ipcRenderer.removeListener("dns:window-max-changed", listener as (...a: unknown[]) => void);
  },
  onOpenAddProfile: (cb: () => void): (() => void) => {
    const listener = () => cb();
    ipcRenderer.on("dns:open-add-profile", listener);
    return () => ipcRenderer.removeListener("dns:open-add-profile", listener);
  },

  onNotification: (cb: (n: BackendNotification) => void): (() => void) => {
    const listener = (_e: unknown, n: BackendNotification) => cb(n);
    ipcRenderer.on("dns:notification", listener as (...a: unknown[]) => void);
    return () => ipcRenderer.removeListener("dns:notification", listener as (...a: unknown[]) => void);
  },
  onBackendReady: (cb: (meta: Record<string, unknown>) => void): (() => void) => {
    const listener = (_e: unknown, m: Record<string, unknown>) => cb(m);
    ipcRenderer.on("dns:backend-ready", listener as (...a: unknown[]) => void);
    return () => ipcRenderer.removeListener("dns:backend-ready", listener as (...a: unknown[]) => void);
  },
  onBackendFatal: (cb: () => void): (() => void) => {
    const listener = () => cb();
    ipcRenderer.on("dns:backend-fatal", listener);
    return () => ipcRenderer.removeListener("dns:backend-fatal", listener);
  },
};

export type DnsApi = typeof api;

contextBridge.exposeInMainWorld("dnsApi", api);

declare global {
  interface Window {
    dnsApi: DnsApi;
  }
}

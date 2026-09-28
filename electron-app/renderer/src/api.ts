// Typed wrapper over window.dnsApi (injected by electron/preload.ts).
// All responses are validated/normalised here so components deal only with
// well-typed data. Throws Error with the backend message on failure.

import type {
  AppSettings,
  BackendMeta,
  BenchmarkEntry,
  CurrentDns,
  DnsProfile,
  NetworkAdapter,
  ProfileFormFields,
} from "./types";

declare global {
  interface Window {
    dnsApi: Record<string, (...args: never[]) => Promise<Record<string, unknown>>>;
  }
}

function api() {
  if (!window.dnsApi) throw new Error("Backend bridge unavailable — restart the app.");
  return window.dnsApi;
}

function asArray<T>(v: unknown): T[] {
  return Array.isArray(v) ? (v as T[]) : [];
}

function str(v: unknown): string {
  return typeof v === "string" ? v : "";
}

function bool(v: unknown): boolean {
  return v === true;
}

export function normAdapter(a: Record<string, unknown>): NetworkAdapter {
  return {
    id: str(a["id"]),
    name: str(a["name"]),
    friendlyName: str(a["friendlyName"]) || str(a["name"]),
    type: str(a["type"]),
    typeLabel: str(a["typeLabel"]) || str(a["type"]),
    enabled: bool(a["enabled"]),
    connected: bool(a["connected"]),
    dhcpEnabled: bool(a["dhcpEnabled"]),
    ipv4: asArray<string>(a["ipv4"]),
    ipv6: asArray<string>(a["ipv6"]),
    dnsServers: asArray<string>(a["dnsServers"]),
    selected: bool(a["selected"]),
  };
}

export function normProfile(p: Record<string, unknown>): DnsProfile {
  const fav = bool(p["isFavorite"]) || bool(p["favorite"]);
  return {
    id: str(p["id"]),
    name: str(p["name"]),
    provider: str(p["provider"]),
    description: str(p["description"]),
    primaryIpv4: str(p["primaryIpv4"] ?? p["primary_ipv4"]),
    secondaryIpv4: str(p["secondaryIpv4"] ?? p["secondary_ipv4"]),
    primaryIpv6: str(p["primaryIpv6"] ?? p["primary_ipv6"]),
    secondaryIpv6: str(p["secondaryIpv6"] ?? p["secondary_ipv6"]),
    primary_ipv4: str(p["primary_ipv4"] ?? p["primaryIpv4"]),
    secondary_ipv4: str(p["secondary_ipv4"] ?? p["secondaryIpv4"]),
    primary_ipv6: str(p["primary_ipv6"] ?? p["primaryIpv6"]),
    secondary_ipv6: str(p["secondary_ipv6"] ?? p["secondaryIpv6"]),
    favorite: fav,
    isFavorite: fav,
    summary: str(p["summary"]),
  };
}

export function normCurrentDns(c: Record<string, unknown>): CurrentDns {
  return {
    adapterId: str(c["adapterId"]),
    adapterName: str(c["adapterName"]),
    available: bool(c["available"]),
    connected: bool(c["connected"]),
    success: bool(c["success"]),
    message: str(c["message"]),
    primaryIpv4: str(c["primaryIpv4"]),
    secondaryIpv4: str(c["secondaryIpv4"]),
    primaryIpv6: str(c["primaryIpv6"]),
    secondaryIpv6: str(c["secondaryIpv6"]),
    isDhcp: bool(c["isDhcp"]),
  };
}

export function normSettings(s: Record<string, unknown>): AppSettings {
  const theme = s["themeMode"];
  return {
    themeMode: theme === "light" || theme === "dark" ? theme : "system",
    startWithWindows: bool(s["startWithWindows"]),
    startMinimized: bool(s["startMinimized"]),
    minimizeToTray: bool(s["minimizeToTray"]),
    flushCacheAfterApply: bool(s["flushCacheAfterApply"]),
    confirmBeforeApply: bool(s["confirmBeforeApply"]),
    settingsFilePath: str(s["settingsFilePath"]),
  };
}

export function normMeta(m: Record<string, unknown>): BackendMeta {
  return {
    version: str(m["version"]) || "1.0.0",
    platform: str(m["platform"]) || "Windows",
    qtVersion: str(m["qtVersion"]),
    profilesFilePath: str(m["profilesFilePath"]),
  };
}

export function normBenchmark(b: Record<string, unknown>): {
  latencyMs: number;
  bestServer: string;
  results: BenchmarkEntry[];
} {
  return {
    latencyMs: typeof b["latencyMs"] === "number" ? b["latencyMs"] : -1,
    bestServer: str(b["bestServer"]),
    results: asArray<Record<string, unknown>>(b["results"]).map((r) => ({
      server: str(r["server"]),
      ok: bool(r["ok"]),
      latencyMs: typeof r["latencyMs"] === "number" ? r["latencyMs"] : -1,
      message: str(r["message"]),
    })),
  };
}

function stateOf(
  r: Record<string, unknown>,
): {
  adapters: NetworkAdapter[];
  selectedAdapterId: string;
  currentDns: CurrentDns | null;
  profiles: DnsProfile[];
  settings: AppSettings | null;
  meta: BackendMeta | null;
} {
  const state = (r["state"] as Record<string, unknown> | undefined) ?? r;
  return {
    adapters: asArray<Record<string, unknown>>(state["adapters"]).map(normAdapter),
    selectedAdapterId: str(state["selectedAdapterId"]),
    currentDns: state["currentDns"]
      ? normCurrentDns(state["currentDns"] as Record<string, unknown>)
      : null,
    profiles: asArray<Record<string, unknown>>(state["profiles"]).map(normProfile),
    settings: state["settings"]
      ? normSettings(state["settings"] as Record<string, unknown>)
      : null,
    meta: state["meta"] ? normMeta(state["meta"] as Record<string, unknown>) : null,
  };
}

export const backend = {
  stateOf,
  normProfile,
  normSettings,
  async getState(search = "") {
    const b = api();
    const r = await (b["getState"] as (s: string) => Promise<Record<string, unknown>>)(search);
    return stateOf(r);
  },
  async refresh() {
    const r = await api()["refresh"]();
    return stateOf(r);
  },
  async selectAdapter(id: string) {
    const r = await (api()["selectAdapter"] as (id: string) => Promise<Record<string, unknown>>)(id);
    return stateOf(r);
  },
  async applyProfile(id: string) {
    const r = await (api()["applyProfile"] as (id: string) => Promise<Record<string, unknown>>)(id);
    return stateOf(r);
  },
  async resetDns() {
    const r = await api()["resetDns"]();
    return stateOf(r);
  },
  async flushCache() {
    await api()["flushCache"]();
  },
  async benchmark() {
    const r = await api()["benchmark"]();
    const bench = normBenchmark((r["benchmark"] as Record<string, unknown>) ?? {});
    return { bench, state: stateOf(r) };
  },
  async listProfiles(search = "") {
    const b = api();
    const r = await (b["listProfiles"] as (s: string) => Promise<Record<string, unknown>>)(search);
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async addProfile(fields: ProfileFormFields) {
    const b = api();
    const r = await (
      b["addProfile"] as (f: unknown) => Promise<Record<string, unknown>>
    )(fields);
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async updateProfile(fields: ProfileFormFields) {
    const b = api();
    const r = await (
      b["updateProfile"] as (f: unknown) => Promise<Record<string, unknown>>
    )(fields);
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async removeProfile(id: string) {
    const r = await (api()["removeProfile"] as (id: string) => Promise<Record<string, unknown>>)(id);
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async toggleFavorite(id: string) {
    const r = await (api()["toggleFavorite"] as (id: string) => Promise<Record<string, unknown>>)(id);
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async resetConfiguration() {
    const r = await api()["resetConfiguration"]();
    return stateOf(r);
  },
  async importDialog() {
    const r = await api()["importDialog"]();
    if (r["canceled"]) return null;
    return asArray<Record<string, unknown>>(r["profiles"]).map(normProfile);
  },
  async exportDialog() {
    const r = await api()["exportDialog"]();
    return r["canceled"] !== true;
  },
  async updateSettings(patch: Partial<AppSettings>) {
    const b = api();
    const r = await (
      b["updateSettings"] as (p: unknown) => Promise<Record<string, unknown>>
    )(patch);
    const s = normSettings((r["settings"] as Record<string, unknown>) ?? {});
    await (b["syncTrayPref"] as unknown as (v: boolean) => Promise<void>)(s.minimizeToTray);
    return s;
  },
};

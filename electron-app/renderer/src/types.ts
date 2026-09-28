// Shared renderer types — mirror the dns-core JSON contract (snake_case kept
// where the C++ side is canonical; camelCase aliases included for DX).

export type NoticeKind = "info" | "success" | "warning" | "error";

export interface Notice {
  id: number;
  text: string;
  kind: NoticeKind;
}

export interface NetworkAdapter {
  id: string;
  name: string;
  friendlyName: string;
  type: string;
  typeLabel: string;
  enabled: boolean;
  connected: boolean;
  dhcpEnabled: boolean;
  ipv4: string[];
  ipv6: string[];
  dnsServers: string[];
  selected: boolean;
}

export interface DnsProfile {
  id: string;
  name: string;
  provider: string;
  description: string;
  primaryIpv4: string;
  secondaryIpv4: string;
  primaryIpv6: string;
  secondaryIpv6: string;
  /** canonical snake_case from C++ (file format) */
  primary_ipv4: string;
  secondary_ipv4: string;
  primary_ipv6: string;
  secondary_ipv6: string;
  favorite: boolean;
  isFavorite: boolean;
  summary: string;
}

export interface CurrentDns {
  adapterId: string;
  adapterName: string;
  available: boolean;
  connected: boolean;
  success: boolean;
  message: string;
  primaryIpv4: string;
  secondaryIpv4: string;
  primaryIpv6: string;
  secondaryIpv6: string;
  isDhcp: boolean;
}

export interface AppSettings {
  themeMode: "system" | "light" | "dark";
  startWithWindows: boolean;
  startMinimized: boolean;
  minimizeToTray: boolean;
  flushCacheAfterApply: boolean;
  confirmBeforeApply: boolean;
  settingsFilePath: string;
}

export interface BackendMeta {
  version: string;
  platform: string;
  qtVersion: string;
  profilesFilePath: string;
}

export interface BenchmarkEntry {
  server: string;
  ok: boolean;
  latencyMs: number;
  message: string;
}

export type PageKey = "dashboard" | "profiles" | "network" | "settings";

export interface ProfileFormFields {
  id?: string;
  name: string;
  provider: string;
  description: string;
  primaryIpv4: string;
  secondaryIpv4: string;
  primaryIpv6: string;
  secondaryIpv6: string;
  isFavorite: boolean;
}

export const emptyProfileForm: ProfileFormFields = {
  name: "",
  provider: "",
  description: "",
  primaryIpv4: "",
  secondaryIpv4: "",
  primaryIpv6: "",
  secondaryIpv6: "",
  isFavorite: false,
};

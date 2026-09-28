// Global app store: backend snapshot + navigation + notices + dialogs.
// All backend mutations funnel through here so every page stays in sync.

import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from "react";
import { backend } from "./api";
import type {
  AppSettings,
  BackendMeta,
  BenchmarkEntry,
  CurrentDns,
  DnsProfile,
  NetworkAdapter,
  Notice,
  NoticeKind,
  PageKey,
  ProfileFormFields,
} from "./types";

interface AppState {
  ready: boolean;
  fatal: string | null;
  booting: boolean;
  refreshing: boolean;
  benchmarking: boolean;
  mutating: boolean;
  adapters: NetworkAdapter[];
  selectedAdapterId: string;
  currentDns: CurrentDns | null;
  profiles: DnsProfile[];
  settings: AppSettings | null;
  meta: BackendMeta | null;
  activeProfileId: string;
  benchmark: { latencyMs: number; bestServer: string; results: BenchmarkEntry[] } | null;
  page: PageKey;
  sidebarCollapsed: boolean;
  notices: Notice[];
  // profile dialog: null = closed, {mode} = open
  profileDialog: { mode: "add" } | { mode: "edit"; profile: DnsProfile } | null;
  confirmDialog: {
    title: string;
    message: string;
    confirmLabel: string;
    danger: boolean;
    action: () => Promise<void>;
  } | null;
  latencyMs: number;
}

interface AppActions {
  setPage: (p: PageKey) => void;
  toggleSidebar: () => void;
  notify: (text: string, kind?: NoticeKind) => void;
  dismissNotice: (id: number) => void;
  refresh: () => Promise<void>;
  selectAdapter: (id: string) => Promise<void>;
  applyProfile: (id: string) => Promise<void>;
  applyActive: () => Promise<void>;
  resetDns: () => Promise<void>;
  flushCache: () => Promise<void>;
  runBenchmark: () => Promise<void>;
  saveProfile: (fields: ProfileFormFields) => Promise<boolean>;
  deleteProfile: (id: string) => Promise<void>;
  toggleFavorite: (id: string) => Promise<void>;
  importProfiles: () => Promise<void>;
  exportProfiles: () => Promise<void>;
  resetConfiguration: () => Promise<void>;
  updateSettings: (patch: Partial<AppSettings>) => Promise<void>;
  openAddProfile: () => void;
  openEditProfile: (p: DnsProfile) => void;
  closeProfileDialog: () => void;
  askConfirm: (c: NonNullable<AppState["confirmDialog"]>) => void;
  closeConfirm: () => void;
  searchProfiles: (q: string) => Promise<void>;
}

const AppCtx = createContext<(AppState & AppActions) | null>(null);

let noticeId = 1;

const initialSettings: AppSettings = {
  themeMode: "system",
  startWithWindows: false,
  startMinimized: false,
  minimizeToTray: true,
  flushCacheAfterApply: true,
  confirmBeforeApply: true,
  settingsFilePath: "",
};

export function AppProvider({ children }: { children: ReactNode }) {
  const [state, setState] = useState<AppState>({
    ready: false,
    fatal: null,
    booting: true,
    refreshing: false,
    benchmarking: false,
    mutating: false,
    adapters: [],
    selectedAdapterId: "",
    currentDns: null,
    profiles: [],
    settings: null,
    meta: null,
    activeProfileId: "",
    benchmark: null,
    page: "dashboard",
    sidebarCollapsed: localStorage.getItem("dnsmgr:sidebar") === "collapsed",
    notices: [],
    profileDialog: null,
    confirmDialog: null,
    latencyMs: -1,
  });
  const stateRef = useRef(state);
  stateRef.current = state;

  const patch = useCallback((p: Partial<AppState>) => {
    setState((s) => ({ ...s, ...p }));
  }, []);

  const notify = useCallback(
    (text: string, kind: NoticeKind = "info") => {
      const id = noticeId++;
      setState((s) => ({ ...s, notices: [...s.notices.slice(-3), { id, text, kind }] }));
      window.setTimeout(() => {
        setState((s) => ({ ...s, notices: s.notices.filter((n) => n.id !== id) }));
      }, 4200);
    },
    [],
  );

  const dismissNotice = useCallback((id: number) => {
    setState((s) => ({ ...s, notices: s.notices.filter((n) => n.id !== id) }));
  }, []);

  const applySnapshot = useCallback(
    (
      snap: Awaited<ReturnType<typeof backend.getState>>,
      extra?: Partial<AppState>,
    ) => {
      patch({
        adapters: snap.adapters,
        selectedAdapterId: snap.selectedAdapterId,
        currentDns: snap.currentDns,
        profiles: snap.profiles,
        settings: snap.settings ?? stateRef.current.settings ?? initialSettings,
        meta: snap.meta ?? stateRef.current.meta,
        latencyMs:
          snap.currentDns && !snap.currentDns.available
            ? -1
            : stateRef.current.latencyMs,
        ...extra,
      });
    },
    [patch],
  );

  const fail = useCallback(
    (e: unknown, fallback: string) => {
      notify(e instanceof Error ? e.message : fallback, "error");
    },
    [notify],
  );

  // --- boot + backend event subscriptions ----------------------------------
  useEffect(() => {
    let cancelled = false;
    backend
      .getState()
      .then((snap) => {
        if (cancelled) return;
        applySnapshot(snap, { ready: true, booting: false });
      })
      .catch((e) => {
        if (cancelled) return;
        patch({ booting: false, fatal: e instanceof Error ? e.message : String(e) });
      });
    const off1 = window.dnsApi?.["onNotification"]
      ? (window.dnsApi as unknown as {
          onNotification: (cb: (n: { text: string; kind: NoticeKind }) => void) => () => void;
        }).onNotification((n) => notify(n.text, n.kind))
      : () => undefined;
    const off2 = window.dnsApi?.["onBackendFatal"]
      ? (window.dnsApi as unknown as { onBackendFatal: (cb: () => void) => () => void }).onBackendFatal(
          () => patch({ fatal: "The DNS backend stopped unexpectedly." }),
        )
      : () => undefined;
    return () => {
      cancelled = true;
      off1();
      off2();
    };
  }, [applySnapshot, notify, patch]);

  useEffect(() => {
    const mode = state.settings?.themeMode ?? "system";
    const mq = window.matchMedia?.("(prefers-color-scheme: light)");
    const apply = () => {
      const light =
        mode === "light" || (mode === "system" && !!mq?.matches);
      document.documentElement.classList.toggle("light", light);
      document.documentElement.classList.toggle("dark", !light);
    };
    apply();
    if (mode === "system" && mq) {
      const onChange = () => apply();
      mq.addEventListener("change", onChange);
      return () => mq.removeEventListener("change", onChange);
    }
  }, [state.settings?.themeMode]);

  // --- actions --------------------------------------------------------------
  const refresh = useCallback(async () => {
    patch({ refreshing: true });
    try {
      applySnapshot(await backend.refresh());
    } catch (e) {
      fail(e, "Refresh failed");
    } finally {
      patch({ refreshing: false });
    }
  }, [applySnapshot, fail, patch]);

  const selectAdapter = useCallback(
    async (id: string) => {
      try {
        applySnapshot(await backend.selectAdapter(id), { benchmark: null, latencyMs: -1 });
      } catch (e) {
        fail(e, "Could not select adapter");
      }
    },
    [applySnapshot, fail],
  );

  const applyProfile = useCallback(
    async (id: string) => {
      const prof = stateRef.current.profiles.find((p) => p.id === id);
      patch({ mutating: true, activeProfileId: id });
      try {
        const snap = await backend.applyProfile(id);
        applySnapshot(snap);
        if (prof) {
          const ok = snap.currentDns?.available;
          if (ok) patch({ activeProfileId: id });
        }
      } catch (e) {
        fail(e, "Could not apply profile");
      } finally {
        patch({ mutating: false });
      }
    },
    [applySnapshot, fail, patch],
  );

  const applyActive = useCallback(async () => {
    const id = stateRef.current.activeProfileId;
    if (!id) {
      notify("Select a profile to apply first", "info");
      return;
    }
    await applyProfile(id);
  }, [applyProfile, notify]);

  const resetDns = useCallback(async () => {
    patch({ mutating: true });
    try {
      applySnapshot(await backend.resetDns());
    } catch (e) {
      fail(e, "Could not reset DNS");
    } finally {
      patch({ mutating: false });
    }
  }, [applySnapshot, fail, patch]);

  const flushCache = useCallback(async () => {
    try {
      await backend.flushCache();
      notify("DNS resolver cache flushed", "success");
    } catch (e) {
      fail(e, "Could not flush the DNS cache");
    }
  }, [fail, notify]);

  const runBenchmark = useCallback(async () => {
    patch({ benchmarking: true });
    try {
      const { bench, state: snap } = await backend.benchmark();
      applySnapshot(snap, { benchmark: bench, latencyMs: bench.latencyMs });
    } catch (e) {
      fail(e, "Benchmark failed");
    } finally {
      patch({ benchmarking: false });
    }
  }, [applySnapshot, fail, patch]);

  const saveProfile = useCallback(
    async (fields: ProfileFormFields): Promise<boolean> => {
      try {
        const profiles = fields.id
          ? await backend.updateProfile(fields)
          : await backend.addProfile(fields);
        patch({ profiles, profileDialog: null });
        notify(fields.id ? "Profile updated" : "Profile saved", "success");
        return true;
      } catch (e) {
        fail(e, "Could not save profile");
        return false;
      }
    },
    [fail, notify, patch],
  );

  const deleteProfile = useCallback(
    async (id: string) => {
      try {
        patch({ profiles: await backend.removeProfile(id) });
      } catch (e) {
        fail(e, "Could not delete profile");
      }
    },
    [fail, patch],
  );

  const toggleFavorite = useCallback(
    async (id: string) => {
      try {
        patch({ profiles: await backend.toggleFavorite(id) });
      } catch (e) {
        fail(e, "Could not update favorite");
      }
    },
    [fail, patch],
  );

  const importProfiles = useCallback(async () => {
    try {
      const profiles = await backend.importDialog();
      if (profiles) {
        patch({ profiles });
        notify("Profiles imported", "success");
      }
    } catch (e) {
      fail(e, "Could not import profiles");
    }
  }, [fail, notify, patch]);

  const exportProfiles = useCallback(async () => {
    try {
      if (await backend.exportDialog()) notify("Profiles exported", "success");
    } catch (e) {
      fail(e, "Could not export profiles");
    }
  }, [fail, notify]);

  const resetConfiguration = useCallback(async () => {
    try {
      const snap = await backend.resetConfiguration();
      applySnapshot(snap, { benchmark: null, latencyMs: -1, activeProfileId: "" });
    } catch (e) {
      fail(e, "Could not reset configuration");
    }
  }, [applySnapshot, fail]);

  const updateSettings = useCallback(
    async (settingsPatch: Partial<AppSettings>) => {
      try {
        patch({ settings: await backend.updateSettings(settingsPatch) });
      } catch (e) {
        fail(e, "Could not save settings");
      }
    },
    [fail, patch],
  );

  const searchProfiles = useCallback(
    async (q: string) => {
      try {
        patch({ profiles: await backend.listProfiles(q) });
      } catch (e) {
        fail(e, "Search failed");
      }
    },
    [fail, patch],
  );

  const setPage = useCallback((page: PageKey) => patch({ page }), [patch]);
  const toggleSidebar = useCallback(() => {
    setState((s) => {
      const collapsed = !s.sidebarCollapsed;
      localStorage.setItem("dnsmgr:sidebar", collapsed ? "collapsed" : "expanded");
      return { ...s, sidebarCollapsed: collapsed };
    });
  }, []);

  const value = useMemo<AppState & AppActions>(
    () => ({
      ...state,
      setPage,
      toggleSidebar,
      notify,
      dismissNotice,
      refresh,
      selectAdapter,
      applyProfile,
      applyActive,
      resetDns,
      flushCache,
      runBenchmark,
      saveProfile,
      deleteProfile,
      toggleFavorite,
      importProfiles,
      exportProfiles,
      resetConfiguration,
      updateSettings,
      openAddProfile: () => patch({ profileDialog: { mode: "add" } }),
      openEditProfile: (profile: DnsProfile) => patch({ profileDialog: { mode: "edit", profile } }),
      closeProfileDialog: () => patch({ profileDialog: null }),
      askConfirm: (c) => patch({ confirmDialog: c }),
      closeConfirm: () => patch({ confirmDialog: null }),
      searchProfiles,
    }),
    [
      state,
      setPage,
      toggleSidebar,
      notify,
      dismissNotice,
      refresh,
      selectAdapter,
      applyProfile,
      applyActive,
      resetDns,
      flushCache,
      runBenchmark,
      saveProfile,
      deleteProfile,
      toggleFavorite,
      importProfiles,
      exportProfiles,
      resetConfiguration,
      updateSettings,
      searchProfiles,
      patch,
    ],
  );

  return <AppCtx.Provider value={value}>{children}</AppCtx.Provider>;
}

export function useApp(): AppState & AppActions {
  const ctx = useContext(AppCtx);
  if (!ctx) throw new Error("useApp must be used inside AppProvider");
  return ctx;
}

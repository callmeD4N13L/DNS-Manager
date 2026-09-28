// Application shell: custom title bar + sidebar + top bar + animated page.

import { AnimatePresence, motion } from "framer-motion";
import { AlertTriangle } from "lucide-react";
import { useCallback, useEffect, useState } from "react";
import logoUrl from "./assets/dancy.png";
import { Sidebar } from "./components/Sidebar";
import { TitleBar } from "./components/TitleBar";
import { TopBar } from "./components/TopBar";
import { StatusBar, Toasts } from "./components/chrome";
import { CommandPalette } from "./components/CommandPalette";
import { CloseDialog, ConfirmDialog, ProfileDialog } from "./components/dialogs";
import { Dashboard } from "./pages/Dashboard";
import { Network } from "./pages/Network";
import { Profiles } from "./pages/Profiles";
import { Settings } from "./pages/Settings";
import { useApp } from "./store";

function BootScreen({ message }: { message: string }) {
  return (
    <div className="flex h-full flex-col items-center justify-center gap-3 bg-abyss">
      <motion.img
        src={logoUrl}
        alt="Dancy"
        initial={{ scale: 0.85, opacity: 0 }}
        animate={{ scale: 1, opacity: 1 }}
        transition={{ type: "spring", stiffness: 300, damping: 22 }}
        className="h-12 w-12 rounded-2xl object-cover shadow-[0_0_28px_-6px_rgba(34,211,238,0.8)]"
        draggable={false}
      />
      <div className="text-[14px] font-semibold text-ink">Dancy</div>
      <div className="font-mono text-[11.5px] text-ink-faint">{message}</div>
    </div>
  );
}

function dnsApi(): Record<string, (...a: never[]) => Promise<unknown>> | undefined {
  return (window as unknown as { dnsApi?: Record<string, (...a: never[]) => Promise<unknown>> }).dnsApi;
}

export function App() {
  const { page, booting, fatal, ready, settings, openAddProfile } = useApp();
  const [palette, setPalette] = useState(false);
  const [closeOpen, setCloseOpen] = useState(false);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "k") {
        e.preventDefault();
        setPalette((v) => !v);
      }
      // Alt+1..4 quick navigation (mirrors sidebar shortcuts).
      if (e.altKey && ["1", "2", "3", "4"].includes(e.key)) {
        e.preventDefault();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  // Tray "Add profile…" deep-link: open the window's profile dialog.
  useEffect(() => {
    const api = dnsApi();
    const sub = (api?.["onOpenAddProfile"] as unknown as (cb: () => void) => () => void | undefined)?.(() =>
      openAddProfile(),
    );
    return () => sub?.();
  }, [openAddProfile]);

  const requestClose = useCallback(() => {
    const remembered = localStorage.getItem("dnsmgr:close-action");
    if (remembered === "minimize") {
      void dnsApi()?.["hideWindow"]?.();
      return;
    }
    if (remembered === "quit") {
      void dnsApi()?.["quitApp"]?.();
      return;
    }
    setCloseOpen(true);
  }, []);

  const doMinimize = useCallback(() => {
    setCloseOpen(false);
    void dnsApi()?.["hideWindow"]?.();
  }, []);

  const doQuit = useCallback(() => {
    setCloseOpen(false);
    void dnsApi()?.["quitApp"]?.();
  }, []);

  if (fatal && !ready) {
    return (
      <div className="flex h-full flex-col bg-abyss">
        <TitleBar onRequestClose={requestClose} />
        <div className="min-h-0 flex-1">
          <BootScreen message="" />
        </div>
      </div>
    );
  }

  if (booting && !ready) {
    return (
      <div className="flex h-full flex-col bg-abyss">
        <TitleBar onRequestClose={requestClose} />
        <div className="min-h-0 flex-1">
          <BootScreen message="connecting to dns-core…" />
        </div>
      </div>
    );
  }

  return (
    <div className="flex h-full flex-col bg-abyss text-ink">
      <TitleBar onRequestClose={requestClose} />
      <div className="flex min-h-0 flex-1">
        <Sidebar />
        <div className="flex min-w-0 flex-1 flex-col">
          <TopBar onOpenPalette={() => setPalette(true)} />
          <main className="smooth-scroll min-h-0 flex-1 overflow-y-auto">
            <div className="mx-auto max-w-6xl px-5 py-5">
              {fatal && (
                <motion.div
                  initial={{ opacity: 0, y: -6 }}
                  animate={{ opacity: 1, y: 0 }}
                  className="mb-4 flex items-center gap-2.5 rounded-xl border border-rose/30 bg-rose-wash px-4 py-3 text-[12.5px] text-rose"
                >
                  <AlertTriangle size={15} className="shrink-0" />
                  {fatal}
                </motion.div>
              )}
              <AnimatePresence mode="wait">
                <motion.div
                  key={page}
                  initial={{ opacity: 0, y: 12, scale: 0.995 }}
                  animate={{ opacity: 1, y: 0, scale: 1 }}
                  exit={{ opacity: 0, y: -8, scale: 0.998 }}
                  transition={{ duration: 0.22, ease: [0.22, 1, 0.36, 1] }}
                >
                  {page === "dashboard" && <Dashboard />}
                  {page === "profiles" && <Profiles />}
                  {page === "network" && <Network />}
                  {page === "settings" && <Settings />}
                </motion.div>
              </AnimatePresence>
            </div>
          </main>
        </div>
      </div>
      <StatusBar />
      <Toasts />
      <ProfileDialog />
      <ConfirmDialog />
      <CloseDialog
        open={closeOpen}
        minimizeToTray={settings?.minimizeToTray ?? true}
        onMinimize={doMinimize}
        onQuit={doQuit}
        onCancel={() => setCloseOpen(false)}
      />
      <CommandPalette open={palette} onClose={() => setPalette(false)} />
    </div>
  );
}

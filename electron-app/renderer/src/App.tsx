// Application shell: sidebar + top bar + animated page + status bar.

import { AnimatePresence, motion } from "framer-motion";
import { AlertTriangle } from "lucide-react";
import { useEffect, useState } from "react";
import { Sidebar } from "./components/Sidebar";
import { TopBar } from "./components/TopBar";
import { StatusBar, Toasts } from "./components/chrome";
import { CommandPalette } from "./components/CommandPalette";
import { ConfirmDialog, ProfileDialog } from "./components/dialogs";
import { Dashboard } from "./pages/Dashboard";
import { Network } from "./pages/Network";
import { Profiles } from "./pages/Profiles";
import { Settings } from "./pages/Settings";
import { useApp } from "./store";

function BootScreen({ message }: { message: string }) {
  return (
    <div className="flex h-full flex-col items-center justify-center gap-3 bg-abyss">
      <span className="flex h-12 w-12 items-center justify-center rounded-2xl bg-accent text-xl font-bold text-accent-ink shadow-[0_0_28px_-6px_rgba(34,211,238,0.8)]">
        D
      </span>
      <div className="text-[14px] font-semibold text-ink">DNS Manager</div>
      <div className="font-mono text-[11.5px] text-ink-faint">{message}</div>
    </div>
  );
}

export function App() {
  const { page, booting, fatal, ready } = useApp();
  const [palette, setPalette] = useState(false);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "k") {
        e.preventDefault();
        setPalette((v) => !v);
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  if (fatal && !ready) {
    return (
      <BootScreen message="" />
    );
  }

  if (booting && !ready) {
    return <BootScreen message="connecting to dns-core…" />;
  }

  return (
    <div className="flex h-full flex-col bg-abyss text-ink">
      <div className="flex min-h-0 flex-1">
        <Sidebar />
        <div className="flex min-w-0 flex-1 flex-col">
          <TopBar onOpenPalette={() => setPalette(true)} />
          <main className="min-h-0 flex-1 overflow-y-auto">
            <div className="mx-auto max-w-6xl px-5 py-5">
              {fatal && (
                <div className="mb-4 flex items-center gap-2.5 rounded-xl border border-rose/30 bg-rose-wash px-4 py-3 text-[12.5px] text-rose">
                  <AlertTriangle size={15} className="shrink-0" />
                  {fatal}
                </div>
              )}
              <AnimatePresence mode="wait">
                <motion.div
                  key={page}
                  initial={{ opacity: 0, y: 10, scale: 0.995 }}
                  animate={{ opacity: 1, y: 0, scale: 1 }}
                  exit={{ opacity: 0, y: -6, scale: 0.998 }}
                  transition={{ duration: 0.18, ease: [0.22, 1, 0.36, 1] }}
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
      <CommandPalette open={palette} onClose={() => setPalette(false)} />
    </div>
  );
}

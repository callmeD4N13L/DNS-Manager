// Command palette (Ctrl+K): fuzzy-ish filtered actions + profile/adapter jumps.

import { AnimatePresence, motion } from "framer-motion";
import {
  ArrowRightLeft,
  Droplets,
  Gauge,
  Layers,
  Plus,
  RefreshCw,
  Search,
  Settings,
  Wifi,
  Zap,
} from "lucide-react";
import { useEffect, useMemo, useRef, useState } from "react";
import { useApp } from "../store";
import { cn } from "../utils";

interface Entry {
  group: string;
  label: string;
  hint: string;
  icon: ReactNode;
  run: () => void;
}

import type { ReactNode } from "react";

export function CommandPalette({ open, onClose }: { open: boolean; onClose: () => void }) {
  const app = useApp();
  const [query, setQuery] = useState("");
  const [index, setIndex] = useState(0);
  const inputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (open) {
      setQuery("");
      setIndex(0);
      requestAnimationFrame(() => inputRef.current?.focus());
    }
  }, [open ]);

  const entries: Entry[] = useMemo(() => {
    const list: Entry[] = [
      { group: "Go to", label: "Dashboard", hint: "Overview", icon: <Gauge size={15} />, run: () => app.setPage("dashboard") },
      { group: "Go to", label: "Profiles", hint: "Saved configurations", icon: <Layers size={15} />, run: () => app.setPage("profiles") },
      { group: "Go to", label: "Network", hint: "Adapters", icon: <Wifi size={15} />, run: () => app.setPage("network") },
      { group: "Go to", label: "Settings", hint: "Preferences", icon: <Settings size={15} />, run: () => app.setPage("settings") },
      { group: "Actions", label: "Refresh adapters", hint: "Re-detect network adapters", icon: <RefreshCw size={15} />, run: () => void app.refresh() },
      { group: "Actions", label: "Benchmark current DNS", hint: "Measure resolver latency", icon: <Zap size={15} />, run: () => void app.runBenchmark() },
      { group: "Actions", label: "Flush DNS cache", hint: "ipconfig /flushdns equivalent", icon: <Droplets size={15} />, run: () => void app.flushCache() },
      { group: "Actions", label: "Add DNS profile", hint: "Create a saved configuration", icon: <Plus size={15} />, run: () => app.openAddProfile() },
    ];
    for (const p of app.profiles) {
      list.push({
        group: "Apply profile",
        label: p.name,
        hint: p.summary || p.provider,
        icon: <ArrowRightLeft size={15} />,
        run: () => void app.applyProfile(p.id),
      });
    }
    return list;
  }, [app]);

  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase();
    if (!q) return entries.slice(0, 12);
    return entries.filter((e) => `${e.label} ${e.hint} ${e.group}`.toLowerCase().includes(q)).slice(0, 12);
  }, [entries, query]);

  useEffect(() => setIndex(0), [query]);

  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") onClose();
      else if (e.key === "ArrowDown") {
        e.preventDefault();
        setIndex((i) => (i + 1) % Math.max(filtered.length, 1));
      } else if (e.key === "ArrowUp") {
        e.preventDefault();
        setIndex((i) => (i - 1 + filtered.length) % Math.max(filtered.length, 1));
      } else if (e.key === "Enter" && filtered[index]) {
        const run = filtered[index].run;
        onClose();
        run();
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open, filtered, index, onClose]);

  return (
    <AnimatePresence>
      {open && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          transition={{ duration: 0.14 }}
          className="fixed inset-0 z-[65] flex justify-center bg-void/60 px-4 pt-24 backdrop-blur-[2px]"
          onMouseDown={(e) => {
            if (e.target === e.currentTarget) onClose();
          }}
        >
          <motion.div
            initial={{ opacity: 0, scale: 0.98, y: -8 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.98, y: -8 }}
            transition={{ type: "spring", stiffness: 500, damping: 36 }}
            className="h-fit w-full max-w-lg overflow-hidden rounded-2xl border border-line bg-surface shadow-pop"
          >
            <div className="flex items-center gap-2.5 border-b border-line-soft px-4">
              <Search size={15} className="shrink-0 text-ink-faint" />
              <input
                ref={inputRef}
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                placeholder="Type a command or search…"
                className="h-12 w-full bg-transparent text-[13.5px] text-ink placeholder:text-ink-faint focus:outline-none"
              />
              <kbd className="rounded border border-line bg-overlay px-1.5 py-0.5 font-mono text-[10px] text-ink-dim">
                ESC
              </kbd>
            </div>
            <div className="max-h-80 overflow-y-auto p-1.5">
              {filtered.length === 0 && (
                <div className="px-3 py-6 text-center text-[12.5px] text-ink-faint">
                  No matching commands
                </div>
              )}
              {filtered.map((e, i) => (
                <button
                  key={`${e.group}:${e.label}`}
                  onMouseEnter={() => setIndex(i)}
                  onClick={() => {
                    onClose();
                    e.run();
                  }}
                  className={cn(
                    "flex w-full items-center gap-3 rounded-lg px-3 py-2.5 text-left",
                    i === index ? "bg-accent-wash" : "hover:bg-hover",
                  )}
                >
                  <span className={cn("shrink-0", i === index ? "text-accent" : "text-ink-faint")}>
                    {e.icon}
                  </span>
                  <span className="min-w-0 flex-1">
                    <span className="block truncate text-[13px] font-medium text-ink">{e.label}</span>
                    <span className="block truncate text-[11.5px] text-ink-faint">
                      {e.group} · {e.hint}
                    </span>
                  </span>
                </button>
              ))}
            </div>
          </motion.div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}

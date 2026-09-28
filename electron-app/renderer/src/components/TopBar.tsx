// Top bar: current section, adapter switcher, global actions, palette hint.

import { AnimatePresence, motion } from "framer-motion";
import { Check, ChevronDown, Command, RefreshCw, Search } from "lucide-react";
import { useEffect, useRef, useState } from "react";
import { useApp } from "../store";
import { cn } from "../utils";
import { IconButton } from "./primitives";

const TITLES = {
  dashboard: "Dashboard",
  profiles: "DNS Profiles",
  network: "Network",
  settings: "Settings",
} as const;

export function TopBar({ onOpenPalette }: { onOpenPalette: () => void }) {
  const { page, adapters, selectedAdapterId, selectAdapter, refresh, refreshing } = useApp();
  const [open, setOpen] = useState(false);
  const menuRef = useRef<HTMLDivElement>(null);
  const selected = adapters.find((a) => a.id === selectedAdapterId);

  useEffect(() => {
    if (!open) return;
    const onDown = (e: MouseEvent) => {
      if (!menuRef.current?.contains(e.target as Node)) setOpen(false);
    };
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    window.addEventListener("mousedown", onDown);
    window.addEventListener("keydown", onKey);
    return () => {
      window.removeEventListener("mousedown", onDown);
      window.removeEventListener("keydown", onKey);
    };
  }, [open ]);

  return (
    <header className="flex h-14 shrink-0 items-center gap-3 border-b border-line bg-abyss/80 px-5 backdrop-blur">
      <AnimatePresence mode="wait">
        <motion.span
          key={page}
          initial={{ opacity: 0, y: 4 }}
          animate={{ opacity: 1, y: 0 }}
          exit={{ opacity: 0, y: -4 }}
          transition={{ duration: 0.15 }}
          className="text-[13.5px] font-semibold tracking-tight text-ink"
        >
          {TITLES[page]}
        </motion.span>
      </AnimatePresence>

      <div className="flex-1" />

      {/* command palette trigger */}
      <button
        onClick={onOpenPalette}
        className="hidden h-8 items-center gap-2 rounded-lg border border-line bg-surface px-2.5 text-[12px] text-ink-faint hover:border-ink-faint/50 hover:text-ink-dim md:flex"
        title="Command palette (Ctrl+K)"
      >
        <Search size={13} />
        <span>Search or command…</span>
        <kbd className="flex items-center gap-0.5 rounded border border-line bg-overlay px-1 py-0.5 font-mono text-[10px] text-ink-dim">
          <Command size={10} />K
        </kbd>
      </button>

      {/* adapter switcher */}
      <div className="relative" ref={menuRef}>
        <button
          onClick={() => setOpen((v) => !v)}
          className={cn(
            "flex h-8 max-w-64 items-center gap-2 rounded-lg border border-line bg-surface px-2.5 text-[12.5px] hover:border-ink-faint/50",
            selected?.connected ? "text-ink" : "text-ink-dim",
          )}
          title="Select network adapter"
          aria-haspopup="listbox"
          aria-expanded={open}
        >
          <span
            className={cn(
              "h-1.5 w-1.5 shrink-0 rounded-full",
              selected?.connected ? "bg-mint shadow-[0_0_6px_rgba(52,211,153,0.9)]" : "bg-ink-faint",
            )}
          />
          <span className="truncate font-medium">
            {selected ? selected.friendlyName : "Select adapter"}
          </span>
          <ChevronDown size={14} className="shrink-0 text-ink-faint" />
        </button>
        <AnimatePresence>
          {open && (
            <motion.div
              role="listbox"
              initial={{ opacity: 0, scale: 0.97, y: -4 }}
              animate={{ opacity: 1, scale: 1, y: 0 }}
              exit={{ opacity: 0, scale: 0.97, y: -4 }}
              transition={{ duration: 0.14 }}
              className="absolute right-0 z-40 mt-2 max-h-72 w-72 overflow-y-auto rounded-xl border border-line bg-surface p-1.5 shadow-pop"
            >
              {adapters.length === 0 && (
                <div className="px-3 py-4 text-center text-[12px] text-ink-faint">
                  No adapters detected
                </div>
              )}
              {adapters.map((a) => (
                <button
                  key={a.id}
                  role="option"
                  aria-selected={a.id === selectedAdapterId}
                  onClick={() => {
                    setOpen(false);
                    if (a.id !== selectedAdapterId) void selectAdapter(a.id);
                  }}
                  className={cn(
                    "flex w-full items-center gap-2.5 rounded-lg px-2.5 py-2 text-left hover:bg-hover",
                    a.id === selectedAdapterId && "bg-accent-wash",
                  )}
                >
                  <span
                    className={cn(
                      "h-1.5 w-1.5 shrink-0 rounded-full",
                      a.connected ? "bg-mint" : "bg-ink-faint",
                    )}
                  />
                  <span className="min-w-0 flex-1">
                    <span className="block truncate text-[12.5px] font-medium text-ink">
                      {a.friendlyName}
                    </span>
                    <span className="block truncate font-mono text-[10.5px] text-ink-faint">
                      {a.typeLabel} · {(a.dnsServers[0] ?? "DHCP").toString()}
                    </span>
                  </span>
                  {a.id === selectedAdapterId && (
                    <Check size={14} className="shrink-0 text-accent" />
                  )}
                </button>
              ))}
            </motion.div>
          )}
        </AnimatePresence>
      </div>

      <IconButton label="Refresh adapters" onClick={() => void refresh()}>
        <RefreshCw size={15} className={refreshing ? "animate-spin" : undefined} />
      </IconButton>
    </header>
  );
}

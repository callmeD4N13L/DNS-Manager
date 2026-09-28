// Status bar + toast stack.

import { AnimatePresence, motion } from "framer-motion";
import { AlertCircle, CheckCircle2, Info, TriangleAlert, X } from "lucide-react";
import { useApp } from "../store";
import { cn } from "../utils";
import type { NoticeKind } from "../types";

const ICONS: Record<NoticeKind, typeof Info> = {
  info: Info,
  success: CheckCircle2,
  warning: TriangleAlert,
  error: AlertCircle,
};

const TONES: Record<NoticeKind, string> = {
  info: "border-accent/30 text-accent",
  success: "border-mint/30 text-mint",
  warning: "border-amber/30 text-amber",
  error: "border-rose/30 text-rose",
};

export function Toasts() {
  const { notices, dismissNotice } = useApp();
  return (
    <div className="pointer-events-none fixed bottom-10 left-1/2 z-[60] flex w-full max-w-md -translate-x-1/2 flex-col items-stretch gap-2 px-4">
      <AnimatePresence>
        {notices.map((n) => {
          const Icon = ICONS[n.kind];
          return (
            <motion.div
              key={n.id}
              layout
              initial={{ opacity: 0, y: 14, scale: 0.97 }}
              animate={{ opacity: 1, y: 0, scale: 1 }}
              exit={{ opacity: 0, y: 8, scale: 0.98 }}
              transition={{ type: "spring", stiffness: 420, damping: 32 }}
              className={cn(
                "pointer-events-auto flex items-start gap-2.5 rounded-xl border bg-surface/95 px-3.5 py-3 shadow-pop backdrop-blur",
                TONES[n.kind],
              )}
            >
              <Icon size={16} className="mt-0.5 shrink-0" />
              <span className="flex-1 text-[12.5px] leading-snug text-ink">{n.text}</span>
              <button
                onClick={() => dismissNotice(n.id)}
                aria-label="Dismiss notification"
                className="rounded p-0.5 text-ink-faint hover:bg-hover hover:text-ink"
              >
                <X size={13} />
              </button>
            </motion.div>
          );
        })}
      </AnimatePresence>
    </div>
  );
}

export function StatusBar() {
  const { meta, adapters, currentDns, latencyMs } = useApp();
  const up = adapters.filter((a) => a.connected).length;
  return (
    <footer className="flex h-7 shrink-0 items-center gap-4 border-t border-line bg-abyss px-4 font-mono text-[10.5px] text-ink-faint">
      <span className="flex items-center gap-1.5">
        <span className="h-1.5 w-1.5 rounded-full bg-mint" />
        dns-core · Qt {meta?.qtVersion ?? "…"}
      </span>
      <span>
        {up}/{adapters.length} adapters up
      </span>
      {currentDns && (
        <span className="truncate">
          {currentDns.adapterName} ·{" "}
          {currentDns.isDhcp && !currentDns.primaryIpv4
            ? "DHCP"
            : currentDns.primaryIpv4 || "no DNS"}
          {latencyMs >= 0 && <span className="text-mint"> · {latencyMs} ms</span>}
        </span>
      )}
      <span className="flex-1" />
      <span className="selectable hidden truncate xl:inline">{meta?.profilesFilePath}</span>
      <span>v{meta?.version ?? "…"}</span>
    </footer>
  );
}

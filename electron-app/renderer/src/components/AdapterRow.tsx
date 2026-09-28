// Adapter row: status, addressing, DNS state, selection, context menu.

import { motion } from "framer-motion";
import { Check, RotateCcw } from "lucide-react";
import { useApp } from "../store";
import type { NetworkAdapter } from "../types";
import { cn } from "../utils";
import { useContextMenu } from "./ContextMenu";
import { Badge, Dot } from "./primitives";

export function AdapterRow({ adapter }: { adapter: NetworkAdapter }) {
  const { selectAdapter, selectedAdapterId, askConfirm, resetDns, flushCache } = useApp();
  const menu = useContextMenu();
  const selected = adapter.id === selectedAdapterId;

  return (
    <>
      <motion.div
        layout
        initial={{ opacity: 0, y: 6 }}
        animate={{ opacity: 1, y: 0 }}
        whileHover={{ y: -2 }}
        transition={{ duration: 0.18, ease: [0.22, 1, 0.36, 1] }}
        role="button"
        tabIndex={0}
        onClick={() => {
          if (!selected) void selectAdapter(adapter.id);
        }}
        onKeyDown={(e) => {
          if ((e.key === "Enter" || e.key === " ") && !selected) {
            e.preventDefault();
            void selectAdapter(adapter.id);
          }
        }}
        onContextMenu={(e) =>
          menu.open(e, [
            { label: "Select adapter", icon: <Check size={14} />, disabled: selected, onSelect: () => void selectAdapter(adapter.id) },
            { label: "Reset DNS to DHCP", icon: <RotateCcw size={14} />, onSelect: () => askConfirm({
              title: "Reset DNS to DHCP",
              message: `Reset DNS on “${adapter.friendlyName}” to automatic (DHCP)?`,
              confirmLabel: "Reset",
              danger: false,
              action: async () => {
                await selectAdapter(adapter.id);
                await resetDns();
              },
            }) },
            { label: "Flush DNS cache", onSelect: () => void flushCache() },
          ])
        }
        className={cn(
          "lift flex cursor-pointer items-center gap-3.5 rounded-xl border px-4 py-3.5 outline-none",
          selected
            ? "border-accent/40 bg-accent-wash/50 shadow-glow"
            : "border-line bg-raised/70 hover:border-ink-faint/40",
        )}
      >
        <Dot color={adapter.connected ? "mint" : "faint"} />
        <div className="min-w-0 flex-1">
          <div className="flex flex-wrap items-center gap-2">
            <span className="truncate text-[13.5px] font-semibold text-ink">
              {adapter.friendlyName}
            </span>
            <Badge tone={adapter.connected ? "mint" : "neutral"}>
              {adapter.connected ? "Connected" : "Disconnected"}
            </Badge>
            <Badge tone="sky">{adapter.typeLabel}</Badge>
            {selected && <Badge tone="accent">Selected</Badge>}
          </div>
          <div className="mt-1.5 flex flex-wrap gap-x-4 gap-y-1 font-mono text-[11px] text-ink-dim">
            {adapter.ipv4[0] && <span className="selectable">IPv4 {adapter.ipv4[0]}</span>}
            {adapter.dnsServers.length > 0 ? (
              <span className="selectable">DNS {adapter.dnsServers.join(", ")}</span>
            ) : (
              <span>DNS automatic (DHCP)</span>
            )}
          </div>
        </div>
      </motion.div>
      {menu.node}
    </>
  );
}

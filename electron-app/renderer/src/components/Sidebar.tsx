// Collapsible app sidebar: icon+label nav, active indicator, count badges,
// tooltips when collapsed, keyboard navigation, persisted state.
// Badge + hover-shortcut patterns ported from OffKnife `dashboard-sidebar`.

import { AnimatePresence, motion } from "framer-motion";
import { Gauge, Layers, ChevronsLeft, ChevronsRight, Settings, Wifi } from "lucide-react";
import { useApp } from "../store";
import { cn } from "../utils";
import type { PageKey } from "../types";

const NAV: Array<{ key: PageKey; label: string; hint: string; icon: typeof Gauge; shortcut: string }> = [
  { key: "dashboard", label: "Dashboard", hint: "Overview & quick actions", icon: Gauge, shortcut: "1" },
  { key: "profiles", label: "Profiles", hint: "Saved DNS configurations", icon: Layers, shortcut: "2" },
  { key: "network", label: "Network", hint: "Adapters & DNS state", icon: Wifi, shortcut: "3" },
  { key: "settings", label: "Settings", hint: "Preferences & data", icon: Settings, shortcut: "4" },
];

export function Sidebar() {
  const { page, setPage, sidebarCollapsed, toggleSidebar, currentDns, profiles, adapters } = useApp();

  const badgeFor = (key: PageKey): string =>
    key === "profiles" && profiles.length > 0
      ? String(profiles.length)
      : key === "network" && adapters.length > 0
        ? String(adapters.length)
        : "";

  return (
    <motion.aside
      initial={false}
      animate={{ width: sidebarCollapsed ? 68 : 224 }}
      transition={{ type: "spring", stiffness: 380, damping: 36 }}
      className="flex h-full shrink-0 flex-col overflow-hidden border-r border-line bg-abyss"
    >
      {/* brand */}
      <div className="flex h-14 shrink-0 items-center gap-2.5 px-4">
        <span className="flex h-8 w-8 shrink-0 items-center justify-center rounded-[10px] bg-accent text-[15px] font-bold text-accent-ink shadow-[0_0_16px_-4px_rgba(34,211,238,0.8)]">
          D
        </span>
        {!sidebarCollapsed && (
          <motion.div
            initial={{ opacity: 0, x: -6 }}
            animate={{ opacity: 1, x: 0 }}
            className="min-w-0"
          >
            <div className="truncate text-[13.5px] font-semibold tracking-tight text-ink">
              DNS Manager
            </div>
            <div className="truncate font-mono text-[10.5px] text-ink-faint">
              {currentDns?.adapterName || "no adapter"}
            </div>
          </motion.div>
        )}
      </div>

      {/* nav */}
      <nav
        aria-label="Primary"
        className="flex flex-1 flex-col gap-1 overflow-y-auto px-3 py-2"
      >
        {!sidebarCollapsed && (
          <div className="px-2 pt-1 pb-1.5 text-[10.5px] font-semibold tracking-[0.08em] text-ink-faint uppercase">
            Manage
          </div>
        )}
        {NAV.map((item, i) => {
          const active = page === item.key;
          const Icon = item.icon;
          const badge = badgeFor(item.key);
          return (
            <button
              key={item.key}
              title={sidebarCollapsed ? `${item.label} — ${item.hint}` : item.hint}
              aria-current={active ? "page" : undefined}
              onClick={() => setPage(item.key)}
              onKeyDown={(e) => {
                if (e.key === "ArrowDown" || e.key === "ArrowUp") {
                  e.preventDefault();
                  const next = (i + (e.key === "ArrowDown" ? 1 : NAV.length - 1)) % NAV.length;
                  setPage(NAV[next].key);
                }
              }}
              className={cn(
                "group relative flex h-10 items-center gap-3 rounded-xl px-3 text-[13px] font-medium",
                active ? "bg-accent-wash text-ink" : "text-ink-dim hover:bg-hover hover:text-ink",
                sidebarCollapsed && "justify-center px-0",
              )}
            >
              {active && (
                <motion.span
                  layoutId="nav-active"
                  transition={{ type: "spring", stiffness: 500, damping: 40 }}
                  className="absolute top-1/2 left-0 h-5 w-[3px] -translate-y-1/2 rounded-full bg-accent shadow-[0_0_8px_rgba(34,211,238,0.9)]"
                />
              )}
              <Icon
                size={17}
                className={cn("shrink-0", active ? "text-accent" : "text-ink-faint group-hover:text-ink-dim")}
              />
              {!sidebarCollapsed && <span className="truncate">{item.label}</span>}
              {/* hover-reveal shortcut (dashboard-sidebar pattern) */}
              {!sidebarCollapsed && !badge && (
                <kbd className="ml-auto hidden rounded border border-line bg-overlay px-1.5 py-0.5 font-mono text-[10px] text-ink-faint group-hover:inline-block">
                  {item.shortcut}
                </kbd>
              )}
              {/* live count badge with pop on change */}
              <AnimatePresence mode="popLayout">
                {badge && (
                  <motion.span
                    key={badge}
                    initial={{ opacity: 0, scale: 0.6 }}
                    animate={{ opacity: 1, scale: 1 }}
                    exit={{ opacity: 0, scale: 0.6 }}
                    transition={{ type: "spring", stiffness: 600, damping: 28 }}
                    className={cn(
                      "flex h-5 min-w-5 items-center justify-center rounded-full bg-accent/15 px-1.5 text-[10.5px] font-semibold text-accent-soft",
                      sidebarCollapsed
                        ? "absolute top-1 right-1 h-2 min-w-2 border border-abyss p-0"
                        : "ml-auto",
                    )}
                  >
                    {!sidebarCollapsed && badge}
                  </motion.span>
                )}
              </AnimatePresence>
            </button>
          );
        })}
      </nav>

      {/* collapse */}
      <div className="shrink-0 border-t border-line-soft p-3">
        <button
          onClick={toggleSidebar}
          title={sidebarCollapsed ? "Expand sidebar" : "Collapse sidebar"}
          aria-label={sidebarCollapsed ? "Expand sidebar" : "Collapse sidebar"}
          className="flex h-9 w-full items-center justify-center gap-2 rounded-xl text-ink-faint hover:bg-hover hover:text-ink"
        >
          {sidebarCollapsed ? <ChevronsRight size={16} /> : (
            <>
              <ChevronsLeft size={16} />
              <span className="text-[12px] font-medium">Collapse</span>
            </>
          )}
        </button>
      </div>
    </motion.aside>
  );
}

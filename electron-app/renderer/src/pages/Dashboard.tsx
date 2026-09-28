// Dashboard: current-DNS hero, status tiles, quick profiles.

import { AnimatePresence, motion } from "framer-motion";
import { Droplets, Layers, Plus, RotateCcw, Zap } from "lucide-react";
import { useApp } from "../store";
import { Badge, Card, Dot, PrimaryButton, SecondaryButton, Skeleton } from "../components/primitives";
import { AnimatedNumber, LatencyGauge, StatCard } from "../components/StatCard";
import { ProfileCard } from "../components/ProfileCard";

function DnsLine({ label, value, mono = true }: { label: string; value: string; mono?: boolean }) {
  return (
    <div>
      <div className="text-[10.5px] font-semibold tracking-[0.07em] text-ink-faint uppercase">
        {label}
      </div>
      <div
        className={
          (mono ? "font-mono " : "") +
          (value ? "text-[15px] font-semibold text-ink selectable" : "text-[13px] text-ink-faint")
        }
      >
        {value || "Not set"}
      </div>
    </div>
  );
}

export function Dashboard() {
  const app = useApp();
  const { currentDns, booting, mutating, benchmarking } = app;
  const activeProfile = app.profiles.find((p) => p.id === app.activeProfileId);

  const needConfirm = (fn: () => void, title: string, message: string, label: string) => {
    if (app.settings?.confirmBeforeApply) {
      app.askConfirm({ title, message, confirmLabel: label, danger: false, action: async () => void fn() });
    } else fn();
  };

  return (
    <div className="flex flex-col gap-5">
      <div className="grid grid-cols-1 gap-5 xl:grid-cols-[380px_1fr]">
        {/* current DNS hero */}
        <Card
          glow
          title="Current DNS"
          subtitle={currentDns?.adapterName || "No adapter selected"}
          action={
            currentDns && (
              <Badge tone={currentDns.connected ? "mint" : "rose"}>
                <Dot color={currentDns.connected ? "mint" : "rose"} />
                {currentDns.connected ? "Connected" : "Disconnected"}
              </Badge>
            )
          }
        >
          {booting || !currentDns ? (
            <div className="flex flex-col gap-3">
              <Skeleton className="h-5 w-2/3" />
              <Skeleton className="h-5 w-1/2" />
              <Skeleton className="h-5 w-2/3" />
            </div>
          ) : (
            <div className="flex flex-col gap-3.5">
              <DnsLine label="Primary IPv4" value={currentDns.primaryIpv4} />
              <DnsLine label="Secondary IPv4" value={currentDns.secondaryIpv4} />
              <DnsLine label="Primary IPv6" value={currentDns.primaryIpv6} />
              <DnsLine label="Secondary IPv6" value={currentDns.secondaryIpv6} />
              <div className="mt-1 flex flex-wrap gap-2">
                <PrimaryButton
                  disabled={!app.activeProfileId || !currentDns.available || mutating}
                  loading={mutating}
                  onClick={() =>
                    needConfirm(
                      () => void app.applyActive(),
                      "Apply profile",
                      `Apply “${activeProfile?.name ?? ""}” to “${currentDns.adapterName}”?`,
                      "Apply",
                    )
                  }
                >
                  Apply{activeProfile ? ` ${activeProfile.name}` : ""}
                </PrimaryButton>
                <SecondaryButton
                  icon={<RotateCcw size={14} />}
                  disabled={!currentDns.available || mutating}
                  onClick={() =>
                    needConfirm(
                      () => void app.resetDns(),
                      "Reset DNS to DHCP",
                      `Reset DNS on “${currentDns.adapterName}” to automatic (DHCP)?`,
                      "Reset",
                    )
                  }
                >
                  DHCP
                </SecondaryButton>
              </div>
            </div>
          )}
        </Card>

        {/* status tiles — OffKnife Stats05 pattern: label + delta + big
            animated value + footer action, staggered spring entrance */}
        <div className="grid grid-cols-1 gap-4 sm:grid-cols-2">
          <StatCard
            index={0}
            label="Connection"
            hint={
              <span className={currentDns?.connected ? "text-mint" : "text-rose"}>
                {currentDns?.connected ? "● live" : "● down"}
              </span>
            }
            value={currentDns?.connected ? "Connected" : "Disconnected"}
            footer={
              <button
                onClick={() => app.setPage("network")}
                className="text-[12px] font-medium text-accent-soft hover:text-accent"
              >
                Inspect adapters →
              </button>
            }
          >
            <p className="truncate text-[12px] text-ink-dim">
              {currentDns?.adapterName ?? "Select an adapter to inspect it."}
            </p>
          </StatCard>

          <StatCard
            index={1}
            label="DNS latency"
            hint={
              app.benchmark?.bestServer ? (
                <span className="max-w-32 truncate font-mono text-[11px] text-ink-faint">
                  via {app.benchmark.bestServer}
                </span>
              ) : (
                <span className="text-ink-faint">not measured</span>
              )
            }
            value={
              <span className="flex items-center gap-3">
                <LatencyGauge latencyMs={app.latencyMs} />
                <span className="font-mono">
                  {benchmarking ? (
                    <span className="text-[16px] font-medium text-ink-dim">probing…</span>
                  ) : (
                    <AnimatedNumber value={Math.max(app.latencyMs, 0)} format={(v) => (app.latencyMs >= 0 ? `${Math.round(v)} ms` : "—")} />
                  )}
                </span>
              </span>
            }
            footer={
              <button
                disabled={!currentDns?.available || benchmarking}
                onClick={() => void app.runBenchmark()}
                className="inline-flex items-center gap-1.5 text-[12px] font-medium text-accent-soft hover:text-accent disabled:cursor-not-allowed disabled:opacity-45"
              >
                <Zap size={13} /> {benchmarking ? "Benchmarking…" : "Run benchmark"}
              </button>
            }
          />

          <StatCard
            index={2}
            label="Active profile"
            hint={
              activeProfile ? (
                <span className="text-accent-soft">● active</span>
              ) : (
                <span className="text-ink-faint">none</span>
              )
            }
            value={
              <span className="block truncate">{activeProfile ? activeProfile.name : "None"}</span>
            }
            footer={
              <button
                onClick={() => app.setPage("profiles")}
                className="text-[12px] font-medium text-accent-soft hover:text-accent"
              >
                Browse profiles →
              </button>
            }
          >
            <p className="truncate text-[12px] text-ink-dim">
              {activeProfile?.summary || "Apply a profile to get started."}
            </p>
          </StatCard>

          <StatCard
            index={3}
            label="Mode"
            hint={
              <span className={currentDns?.isDhcp ? "text-mint" : "text-accent-soft"}>
                {currentDns ? (currentDns.isDhcp ? "automatic" : "static") : "—"}
              </span>
            }
            value={currentDns ? (currentDns.isDhcp ? "DHCP" : "Static DNS") : "—"}
            footer={
              <button
                onClick={() => void app.flushCache()}
                className="inline-flex items-center gap-1.5 text-[12px] font-medium text-accent-soft hover:text-accent"
              >
                <Droplets size={13} /> Flush resolver cache
              </button>
            }
          />
        </div>
      </div>

      {/* quick profiles */}
      <Card
        title="Saved DNS profiles"
        subtitle="One-click switch between your saved configurations"
        action={
          <SecondaryButton icon={<Plus size={14} />} onClick={app.openAddProfile}>
            Add profile
          </SecondaryButton>
        }
      >
        {app.profiles.length === 0 ? (
          <div className="flex flex-col items-center gap-1.5 py-8 text-center">
            <Layers size={28} className="mb-1 text-ink-faint" />
            <div className="text-[13.5px] font-medium text-ink-dim">No profiles yet</div>
            <div className="text-[12px] text-ink-faint">Create your first saved configuration.</div>
          </div>
        ) : (
          <div className="flex flex-col gap-2.5">
            <AnimatePresence initial={false}>
              {app.profiles.slice(0, 6).map((p) => (
                <ProfileCard key={p.id} profile={p} />
              ))}
            </AnimatePresence>
            {app.profiles.length > 6 && (
              <button
                onClick={() => app.setPage("profiles")}
                className="mt-1 text-[12.5px] font-medium text-accent-soft hover:text-accent"
              >
                Show all {app.profiles.length} profiles →
              </button>
            )}
          </div>
        )}
      </Card>

      {/* benchmark detail — animated relative bars, staggered entrance */}
      <AnimatePresence>
        {app.benchmark && app.benchmark.results.length > 0 && (
          <motion.div
            initial={{ opacity: 0, y: 8 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0 }}
          >
            <Card title="Last benchmark" subtitle="Per-server latency of the selected adapter">
              <div className="flex flex-col gap-2.5">
                {(() => {
                  const ok = app.benchmark.results.filter((r) => r.ok);
                  const max = Math.max(1, ...ok.map((r) => r.latencyMs));
                  return app.benchmark.results.map((r, i) => (
                    <div key={r.server}>
                      <div className="mb-1 flex items-center gap-2">
                        <Dot color={r.ok ? (r.latencyMs < 100 ? "mint" : r.latencyMs < 300 ? "amber" : "rose") : "rose"} />
                        <span className="flex-1 truncate font-mono text-[12px] text-ink selectable">{r.server}</span>
                        <span className="font-mono text-[12px] text-ink-dim">
                          {r.ok ? `${r.latencyMs} ms` : r.message || "timeout"}
                        </span>
                      </div>
                      <div className="h-1.5 overflow-hidden rounded-full bg-overlay">
                        <motion.div
                          initial={{ width: 0 }}
                          animate={{ width: r.ok ? `${Math.max(4, (r.latencyMs / max) * 100)}%` : "4%" }}
                          transition={{ delay: 0.08 * i, type: "spring", stiffness: 120, damping: 20 }}
                          className={
                            r.ok
                              ? r.latencyMs < 100
                                ? "h-full rounded-full bg-mint"
                                : r.latencyMs < 300
                                  ? "h-full rounded-full bg-amber"
                                  : "h-full rounded-full bg-rose"
                              : "h-full rounded-full bg-ink-faint"
                          }
                        />
                      </div>
                    </div>
                  ));
                })()}
              </div>
            </Card>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}

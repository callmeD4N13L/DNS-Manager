// Network page: adapter list with selection, reset, flush, refresh.

import { Droplets, RefreshCw, RotateCcw, Wifi } from "lucide-react";
import { AdapterRow } from "../components/AdapterRow";
import { Card, EmptyState, PageHeader, SecondaryButton, Skeleton } from "../components/primitives";
import { useApp } from "../store";

export function Network() {
  const app = useApp();
  const selected = app.adapters.find((a) => a.id === app.selectedAdapterId);

  return (
    <div className="flex flex-col gap-5">
      <PageHeader
        title="Network"
        subtitle="Detected adapters — select the one you want to modify"
        action={
          <>
            <SecondaryButton
              icon={<RotateCcw size={14} />}
              disabled={!selected}
              onClick={() =>
                app.askConfirm({
                  title: "Reset DNS to DHCP",
                  message: `Reset DNS on “${selected?.friendlyName}” to automatic (DHCP)?`,
                  confirmLabel: "Reset",
                  danger: false,
                  action: () => app.resetDns(),
                })
              }
            >
              Reset to DHCP
            </SecondaryButton>
            <SecondaryButton icon={<Droplets size={14} />} onClick={() => void app.flushCache()}>
              Flush cache
            </SecondaryButton>
            <SecondaryButton
              icon={<RefreshCw size={14} className={app.refreshing ? "animate-spin" : undefined} />}
              onClick={() => void app.refresh()}
            >
              Refresh
            </SecondaryButton>
          </>
        }
      />
      <Card title="Adapters" subtitle={`${app.adapters.length} adapter${app.adapters.length === 1 ? "" : "s"} found`}>
        {app.booting ? (
          <div className="flex flex-col gap-2.5">
            <Skeleton className="h-16" />
            <Skeleton className="h-16" />
            <Skeleton className="h-16" />
          </div>
        ) : app.adapters.length === 0 ? (
          <EmptyState
            icon={<Wifi size={22} />}
            title="No adapters detected"
            hint="Check that a network interface is enabled, then refresh."
          />
        ) : (
          <div className="flex flex-col gap-2.5">
            {app.adapters.map((a) => (
              <AdapterRow key={a.id} adapter={a} />
            ))}
          </div>
        )}
      </Card>
    </div>
  );
}

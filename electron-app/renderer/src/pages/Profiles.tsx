// Profiles page: search + full CRUD over the real C++ ProfileManager.

import { Layers, Plus, Search } from "lucide-react";
import { useEffect, useState } from "react";
import { ProfileCard } from "../components/ProfileCard";
import { Card, EmptyState, PageHeader, PrimaryButton, Skeleton } from "../components/primitives";
import { useApp } from "../store";
import { useDebounced } from "../hooks";

export function Profiles() {
  const app = useApp();
  const [query, setQuery] = useState("");
  const debounced = useDebounced(query, 250);

  useEffect(() => {
    void app.searchProfiles(debounced);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [debounced]);

  return (
    <div className="flex flex-col gap-5">
      <PageHeader
        title="DNS Profiles"
        subtitle="Create, edit and switch between saved DNS configurations"
        action={
          <PrimaryButton icon={<Plus size={14} />} onClick={app.openAddProfile}>
            Add profile
          </PrimaryButton>
        }
      />
      <Card
        title="All profiles"
        subtitle={`${app.profiles.length} profile${app.profiles.length === 1 ? "" : "s"}`}
      >
        <div className="mb-3 flex h-10 items-center gap-2 rounded-lg border border-line bg-abyss px-3 focus-within:border-accent/60">
          <Search size={15} className="shrink-0 text-ink-faint" />
          <input
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            placeholder="Search profiles…"
            aria-label="Search profiles"
            className="w-full bg-transparent text-[13px] text-ink placeholder:text-ink-faint focus:outline-none"
          />
        </div>
        {app.booting ? (
          <div className="flex flex-col gap-2.5">
            <Skeleton className="h-20" />
            <Skeleton className="h-20" />
            <Skeleton className="h-20" />
          </div>
        ) : app.profiles.length === 0 ? (
          <EmptyState
            icon={<Layers size={22} />}
            title={query ? "No matching profiles" : "No profiles yet"}
            hint={query ? "Try a different search." : "Click “Add profile” to create your first one."}
          />
        ) : (
          <div className="flex flex-col gap-2.5">
            {app.profiles.map((p) => (
              <ProfileCard key={p.id} profile={p} />
            ))}
          </div>
        )}
      </Card>
    </div>
  );
}

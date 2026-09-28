// Saved-profile card: identity, server summary, favorite + active states,
// apply/edit/delete actions, right-click menu.

import { motion } from "framer-motion";
import { Pencil, Play, Star, Trash2 } from "lucide-react";
import { useApp } from "../store";
import type { DnsProfile } from "../types";
import { cn } from "../utils";
import { useContextMenu } from "./ContextMenu";
import { IconButton } from "./primitives";

export function ProfileCard({ profile }: { profile: DnsProfile }) {
  const { activeProfileId, applyProfile, toggleFavorite, openEditProfile, askConfirm, deleteProfile, mutating } =
    useApp();
  const menu = useContextMenu();
  const isActive = profile.id === activeProfileId;

  return (
    <>
      <motion.div
        layout
        initial={{ opacity: 0, y: 6 }}
        animate={{ opacity: 1, y: 0 }}
        whileHover={{ y: -2 }}
        transition={{ duration: 0.18 }}
        onContextMenu={(e) =>
          menu.open(e, [
            { label: "Apply to selected adapter", icon: <Play size={14} />, onSelect: () => void applyProfile(profile.id) },
            { label: profile.isFavorite ? "Remove favorite" : "Mark favorite", icon: <Star size={14} />, onSelect: () => void toggleFavorite(profile.id) },
            { label: "Edit…", icon: <Pencil size={14} />, onSelect: () => openEditProfile(profile) },
            { label: "Delete", icon: <Trash2 size={14} />, danger: true, onSelect: () => askConfirm({
              title: "Delete profile",
              message: `Delete “${profile.name}”? This cannot be undone.`,
              confirmLabel: "Delete",
              danger: true,
              action: () => deleteProfile(profile.id),
            }) },
          ])
        }
        className={cn(
          "group flex items-center gap-3.5 rounded-xl border bg-raised/70 px-4 py-3 transition-colors",
          isActive ? "border-accent/40 shadow-glow" : "border-line hover:border-ink-faint/40",
        )}
      >
        <button
          onClick={() => void toggleFavorite(profile.id)}
          title={profile.isFavorite ? "Remove favorite" : "Mark favorite"}
          aria-label="Toggle favorite"
          className={cn(
            "shrink-0 rounded-md p-1",
            profile.isFavorite ? "text-amber" : "text-ink-faint opacity-0 group-hover:opacity-100 hover:text-amber",
          )}
        >
          <Star size={15} fill={profile.isFavorite ? "currentColor" : "none"} />
        </button>

        <div className="min-w-0 flex-1">
          <div className="flex items-center gap-2">
            <span className="truncate text-[13.5px] font-semibold text-ink">{profile.name}</span>
            {isActive && (
              <span className="rounded-full border border-accent/30 bg-accent-wash px-2 py-px text-[10.5px] font-semibold text-accent-soft">
                ACTIVE
              </span>
            )}
          </div>
          <div className="truncate text-[11.5px] text-ink-faint">
            {[profile.provider, profile.summary].filter(Boolean).join(" · ") || "—"}
          </div>
          <div className="mt-1 flex flex-wrap gap-x-3 gap-y-0.5 font-mono text-[11px] text-ink-dim">
            {profile.primaryIpv4 && <span className="selectable">{profile.primaryIpv4}</span>}
            {profile.secondaryIpv4 && <span className="selectable">{profile.secondaryIpv4}</span>}
            {profile.primaryIpv6 && <span className="selectable">{profile.primaryIpv6}</span>}
            {profile.secondaryIpv6 && <span className="selectable">{profile.secondaryIpv6}</span>}
          </div>
        </div>

        <div className="flex shrink-0 items-center gap-1 opacity-0 transition-opacity group-hover:opacity-100 focus-within:opacity-100">
          <IconButton label="Edit profile" onClick={() => openEditProfile(profile)}>
            <Pencil size={14} />
          </IconButton>
          <IconButton
            label="Delete profile"
            onClick={() =>
              askConfirm({
                title: "Delete profile",
                message: `Delete “${profile.name}”? This cannot be undone.`,
                confirmLabel: "Delete",
                danger: true,
                action: () => deleteProfile(profile.id),
              })
            }
          >
            <Trash2 size={14} />
          </IconButton>
        </div>

        <button
          onClick={() => void applyProfile(profile.id)}
          disabled={mutating}
          className={cn(
            "flex h-8 shrink-0 items-center gap-1.5 rounded-lg px-3 text-[12.5px] font-semibold disabled:opacity-50",
            isActive
              ? "border border-accent/40 bg-accent-wash text-accent-soft hover:bg-accent-wash"
              : "bg-accent text-accent-ink hover:bg-accent-soft",
          )}
        >
          <Play size={13} />
          Apply
        </button>
      </motion.div>
      {menu.node}
    </>
  );
}

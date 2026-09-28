// Profile editor + generic confirm dialog, rendered from store state.

import { AnimatePresence } from "framer-motion";
import { useEffect, useState } from "react";
import { useApp } from "../store";
import { emptyProfileForm, type ProfileFormFields } from "../types";
import { DangerButton, Field, Modal, PrimaryButton, SecondaryButton, TextInput, Toggle } from "./primitives";

const IPV4 = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;

function validIpv4(s: string): boolean {
  if (!s) return true;
  const m = IPV4.exec(s.trim());
  if (!m) return false;
  return m.slice(1).every((o) => Number(o) <= 255);
}

function validIpv6(s: string): boolean {
  if (!s) return true;
  // Full RFC parse lives in C++; this is a cheap client-side pre-check.
  return /^[0-9a-fA-F:.]{2,45}$/.test(s.trim()) && s.includes(":");
}

function validate(f: ProfileFormFields): Record<string, string> {
  const errors: Record<string, string> = {};
  if (!f.name.trim()) errors["name"] = "A name is required.";
  if (!f.primaryIpv4 && !f.secondaryIpv4 && !f.primaryIpv6 && !f.secondaryIpv6)
    errors["primaryIpv4"] = "At least one DNS address is required.";
  if (!validIpv4(f.primaryIpv4)) errors["primaryIpv4"] = "Invalid IPv4 address.";
  if (!validIpv4(f.secondaryIpv4)) errors["secondaryIpv4"] = "Invalid IPv4 address.";
  if (!validIpv6(f.primaryIpv6)) errors["primaryIpv6"] = "Invalid IPv6 address.";
  if (!validIpv6(f.secondaryIpv6)) errors["secondaryIpv6"] = "Invalid IPv6 address.";
  return errors;
}

export function ProfileDialog() {
  const { profileDialog, closeProfileDialog, saveProfile } = useApp();
  const [form, setForm] = useState<ProfileFormFields>(emptyProfileForm);
  const [errors, setErrors] = useState<Record<string, string>>({});
  const [saving, setSaving] = useState(false);

  useEffect(() => {
    if (profileDialog?.mode === "edit") {
      const p = profileDialog.profile;
      setForm({
        id: p.id,
        name: p.name,
        provider: p.provider,
        description: p.description,
        primaryIpv4: p.primaryIpv4,
        secondaryIpv4: p.secondaryIpv4,
        primaryIpv6: p.primaryIpv6,
        secondaryIpv6: p.secondaryIpv6,
        isFavorite: p.isFavorite,
      });
    } else {
      setForm(emptyProfileForm);
    }
    setErrors({});
    setSaving(false);
  }, [profileDialog]);

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") closeProfileDialog();
    };
    if (profileDialog) window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [profileDialog, closeProfileDialog]);

  const set = (k: keyof ProfileFormFields, v: string | boolean) => {
    setForm((f) => ({ ...f, [k]: v }));
    setErrors((e) => {
      const next = { ...e };
      delete next[k];
      return next;
    });
  };

  const submit = async () => {
    const errs = validate(form);
    setErrors(errs);
    if (Object.keys(errs).length > 0) return;
    setSaving(true);
    const ok = await saveProfile(form);
    setSaving(false);
    if (ok) closeProfileDialog();
  };

  return (
    <AnimatePresence>
      {profileDialog && (
        <Modal
          title={profileDialog.mode === "add" ? "Add DNS profile" : "Edit DNS profile"}
          description="Profiles are validated by the C++ backend before saving."
          onClose={closeProfileDialog}
          wide
          footer={
            <>
              <SecondaryButton onClick={closeProfileDialog}>Cancel</SecondaryButton>
              <PrimaryButton onClick={() => void submit()} loading={saving}>
                {profileDialog.mode === "add" ? "Save profile" : "Save changes"}
              </PrimaryButton>
            </>
          }
        >
          <form
            className="grid grid-cols-1 gap-4 sm:grid-cols-2"
            onSubmit={(e) => {
              e.preventDefault();
              void submit();
            }}
          >
            <div className="sm:col-span-2">
              <Field label="Name" error={errors["name"]}>
                <TextInput
                  autoFocus
                  value={form.name}
                  maxLength={128}
                  placeholder="e.g. Cloudflare"
                  onChange={(e) => set("name", e.target.value)}
                />
              </Field>
            </div>
            <div className="sm:col-span-2">
              <Field label="Provider" hint="Optional — shown under the profile name.">
                <TextInput
                  value={form.provider}
                  maxLength={128}
                  placeholder="e.g. Cloudflare, Inc."
                  onChange={(e) => set("provider", e.target.value)}
                />
              </Field>
            </div>
            <Field label="Primary IPv4" error={errors["primaryIpv4"]} mono>
              <TextInput
                value={form.primaryIpv4}
                placeholder="1.1.1.1"
                spellCheck={false}
                onChange={(e) => set("primaryIpv4", e.target.value)}
              />
            </Field>
            <Field label="Secondary IPv4" error={errors["secondaryIpv4"]} mono>
              <TextInput
                value={form.secondaryIpv4}
                placeholder="1.0.0.1"
                spellCheck={false}
                onChange={(e) => set("secondaryIpv4", e.target.value)}
              />
            </Field>
            <Field label="Primary IPv6" error={errors["primaryIpv6"]} mono>
              <TextInput
                value={form.primaryIpv6}
                placeholder="2606:4700:4700::1111"
                spellCheck={false}
                onChange={(e) => set("primaryIpv6", e.target.value)}
              />
            </Field>
            <Field label="Secondary IPv6" error={errors["secondaryIpv6"]} mono>
              <TextInput
                value={form.secondaryIpv6}
                placeholder="2606:4700:4700::1001"
                spellCheck={false}
                onChange={(e) => set("secondaryIpv6", e.target.value)}
              />
            </Field>
            <div className="flex items-center gap-2.5 sm:col-span-2">
              <Toggle checked={form.isFavorite} onChange={(v) => set("isFavorite", v)} label="Favorite" />
              <span className="text-[12.5px] text-ink-dim">Mark as favorite</span>
            </div>
          </form>
        </Modal>
      )}
    </AnimatePresence>
  );
}

export function CloseDialog({
  open,
  minimizeToTray,
  onMinimize,
  onQuit,
  onCancel,
}: {
  open: boolean;
  minimizeToTray: boolean;
  onMinimize: () => void;
  onQuit: () => void;
  onCancel: () => void;
}) {
  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") onCancel();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open, onCancel]);

  return (
    <AnimatePresence>
      {open && (
        <Modal
          title="Close DNS Manager?"
          description="The app can keep running in the system tray so you can switch profiles or re-open it from the notification area."
          onClose={onCancel}
          footer={
            <>
              <SecondaryButton onClick={onCancel}>Cancel</SecondaryButton>
              <SecondaryButton onClick={onQuit}>Quit app</SecondaryButton>
              <PrimaryButton onClick={onMinimize}>
                {minimizeToTray ? "Minimize to tray" : "Run in background"}
              </PrimaryButton>
            </>
          }
        >
          <p className="text-[13px] leading-relaxed text-ink-dim">
            Minimized to the tray, DNS Manager stays available next to the clock:
            right-click the tray icon to apply a profile or flush the cache, or
            click it to re-open the window.
          </p>
        </Modal>
      )}
    </AnimatePresence>
  );
}

export function ConfirmDialog() {
  const { confirmDialog, closeConfirm } = useApp();
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    setBusy(false);
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") closeConfirm();
    };
    if (confirmDialog) window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [confirmDialog, closeConfirm]);

  return (
    <AnimatePresence>
      {confirmDialog && (
        <Modal
          title={confirmDialog.title}
          onClose={closeConfirm}
          footer={
            <>
              <SecondaryButton onClick={closeConfirm}>Cancel</SecondaryButton>
              {confirmDialog.danger ? (
                <DangerButton
                  loading={busy}
                  onClick={() => {
                    setBusy(true);
                    void confirmDialog.action().finally(() => {
                      setBusy(false);
                      closeConfirm();
                    });
                  }}
                >
                  {confirmDialog.confirmLabel}
                </DangerButton>
              ) : (
                <PrimaryButton
                  loading={busy}
                  onClick={() => {
                    setBusy(true);
                    void confirmDialog.action().finally(() => {
                      setBusy(false);
                      closeConfirm();
                    });
                  }}
                >
                  {confirmDialog.confirmLabel}
                </PrimaryButton>
              )}
            </>
          }
        >
          <p className="text-[13px] leading-relaxed text-ink-dim">{confirmDialog.message}</p>
        </Modal>
      )}
    </AnimatePresence>
  );
}

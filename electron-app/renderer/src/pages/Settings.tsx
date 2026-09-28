// Settings page: appearance, startup/tray, DNS behavior, data, about.
// Every control is wired to the real C++ AppSettings / ProfileManager.

import { Card, PageHeader, SecondaryButton, SettingRow, Toggle } from "../components/primitives";
import { useApp } from "../store";
import { cn } from "../utils";

function ThemeOptions() {
  const { settings, updateSettings } = useApp();
  const modes = [
    { key: "system", label: "System" },
    { key: "dark", label: "Dark" },
    { key: "light", label: "Light" },
  ] as const;
  return (
    <div className="flex gap-1 rounded-lg border border-line bg-abyss p-1" role="radiogroup" aria-label="Theme">
      {modes.map((m) => (
        <button
          key={m.key}
          role="radio"
          aria-checked={settings?.themeMode === m.key}
          onClick={() => void updateSettings({ themeMode: m.key })}
          className={cn(
            "rounded-md px-3.5 py-1.5 text-[12.5px] font-medium",
            settings?.themeMode === m.key
              ? "bg-accent text-accent-ink shadow"
              : "text-ink-dim hover:bg-hover hover:text-ink",
          )}
        >
          {m.label}
        </button>
      ))}
    </div>
  );
}

export function Settings() {
  const app = useApp();
  const s = app.settings;

  return (
    <div className="flex flex-col gap-5">
      <PageHeader title="Settings" subtitle="Application preferences" />

      <Card title="Appearance" subtitle="System follows the Windows light/dark setting">
        <SettingRow label="Theme" description="Applies instantly across the whole app">
          <ThemeOptions />
        </SettingRow>
      </Card>

      <Card title="Startup & tray" subtitle="Launch and background behavior">
        <div className="flex flex-col gap-4">
          <SettingRow label="Start with Windows" description="Launch automatically when you sign in">
            <Toggle checked={!!s?.startWithWindows} onChange={(v) => void app.updateSettings({ startWithWindows: v })} label="Start with Windows" />
          </SettingRow>
          <SettingRow label="Launch minimized" description="Start hidden in the system tray">
            <Toggle checked={!!s?.startMinimized} onChange={(v) => void app.updateSettings({ startMinimized: v })} label="Launch minimized" />
          </SettingRow>
          <SettingRow label="Minimize to system tray" description="Keep running in the tray when the window is closed">
            <Toggle checked={!!s?.minimizeToTray} onChange={(v) => void app.updateSettings({ minimizeToTray: v })} label="Minimize to tray" />
          </SettingRow>
        </div>
      </Card>

      <Card title="DNS behavior" subtitle="What happens after you apply a configuration">
        <div className="flex flex-col gap-4">
          <SettingRow label="Flush DNS cache after applying" description="Run the resolver-cache flush after every successful change">
            <Toggle checked={!!s?.flushCacheAfterApply} onChange={(v) => void app.updateSettings({ flushCacheAfterApply: v })} label="Flush DNS cache after applying" />
          </SettingRow>
          <SettingRow label="Confirm before changing DNS" description="Ask for confirmation before applying or resetting">
            <Toggle checked={!!s?.confirmBeforeApply} onChange={(v) => void app.updateSettings({ confirmBeforeApply: v })} label="Confirm before changing DNS" />
          </SettingRow>
        </div>
      </Card>

      <Card title="Data" subtitle="Profiles and configuration">
        <div className="flex flex-col gap-4">
          <SettingRow label="Import profiles" description="Merge DNS profiles from a JSON file">
            <SecondaryButton onClick={() => void app.importProfiles()}>Import…</SecondaryButton>
          </SettingRow>
          <SettingRow label="Export profiles" description="Save all profiles to a JSON file">
            <SecondaryButton onClick={() => void app.exportProfiles()}>Export…</SecondaryButton>
          </SettingRow>
          <SettingRow label="Reset configuration" description="Restore default profiles and settings">
            <SecondaryButton
              onClick={() =>
                app.askConfirm({
                  title: "Reset configuration",
                  message: "Restore default profiles and settings? Current profiles will be replaced.",
                  confirmLabel: "Reset",
                  danger: true,
                  action: () => app.resetConfiguration(),
                })
              }
            >
              Reset
            </SecondaryButton>
          </SettingRow>
          <SettingRow label="Profiles file" description="Where DNS profiles are stored">
            <span className="selectable max-w-64 truncate font-mono text-[11px] text-ink-dim" title={app.meta?.profilesFilePath}>
              {app.meta?.profilesFilePath}
            </span>
          </SettingRow>
          <SettingRow label="Settings file" description="Where application preferences are stored">
            <span className="selectable max-w-64 truncate font-mono text-[11px] text-ink-dim" title={s?.settingsFilePath}>
              {s?.settingsFilePath}
            </span>
          </SettingRow>
        </div>
      </Card>

      <Card title="About" subtitle="Version information">
        <div className="flex flex-col gap-1.5 text-[12.5px]">
          <div className="flex justify-between">
            <span className="text-ink-dim">Application</span>
            <span className="font-semibold text-ink">DNS Manager {app.meta?.version}</span>
          </div>
          <div className="flex justify-between">
            <span className="text-ink-dim">Backend</span>
            <span className="text-ink-dim">dns-core · C++ · {app.meta?.platform} / Qt {app.meta?.qtVersion}</span>
          </div>
          <p className="mt-1 text-[12px] leading-relaxed text-ink-faint">
            Switch, benchmark and troubleshoot your Windows DNS configuration. Privileged
            operations elevate through the standard Windows UAC prompt — never silently.
          </p>
        </div>
      </Card>
    </div>
  );
}

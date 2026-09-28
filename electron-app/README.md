# Dancy — Electron + React UI over the C++ backend

The entire user interface is rendered by **Electron → Chromium → React +
TypeScript + Tailwind CSS + Framer Motion**. All DNS, networking, security and
system logic stays in C++ and is exposed through a headless sidecar.

```
renderer (React/TS) ──contextBridge──▶ Electron main ──stdio NDJSON──▶ dns-core (C++)
   Chromium UI            preload +            subprocess bridge         core/ models/
                          validated IPC                             platform/ (unchanged)
```

## Layout

| Path | Contents |
| ---- | -------- |
| `electron/main.ts` | Window, tray, single-instance, native dialogs, IPC allowlist |
| `electron/preload.ts` | Frozen `window.dnsApi` bridge (contextIsolation, no Node in renderer) |
| `electron/bridge.ts` | Spawns/supervises `dns-core.exe`, NDJSON JSON-RPC, timeouts, restarts |
| `renderer/src/` | React app: `App.tsx`, `store.tsx`, `api.ts`, `pages/`, `components/`, `theme.css` |
| `scripts/smoke-dnscore.mjs` | Protocol smoke test (read + safe write paths, no system DNS changes) |
| `../src/cli/dnscore_main.cpp` | The sidecar — reuses `core/`, `models/`, `platform/` 1:1 |

## Prerequisites

- Node.js 20+ and npm
- Qt 6.5+ with MSVC (only to build `dns-core`; the UI itself needs no Qt)
- A configured CMake build, e.g. `cmake --preset vs2022-release`
  (or `ninja-release`) so `build/<preset>/dns-core.exe` exists for dev mode.
  Building from a plain shell needs the VS environment — see `scripts/` note
  in the root docs, or run from a "Developer Command Prompt".

## Develop

```powershell
cd electron-app
npm install
npm run dev        # Vite on :5173 + Electron with hot reload
```

Dev mode resolves `dns-core.exe` from `../build/ninja-release/` (falling back
to `vs2022-release` / `vs2026-release`).

## Build & package

```powershell
cd electron-app
npm run build            # tsc (electron) + vite build (renderer)
npm run dist:portable    # 82 MB portable EXE incl. dns-core sidecar
npm run dist             # NSIS installer + portable
npm run smoke:bridge     # sidecar protocol test (19 assertions)
```

`npx electron . --smoke-test` launches the real app headlessly against the
built renderer and exits 0 once React paints live backend data.

## Security

- `contextIsolation: true`, `nodeIntegration: false`, `sandbox: true`
- Renderer receives only the frozen `window.dnsApi` surface — no Node, no
  `require`, no shell
- Main validates every IPC payload (ids, profile fields, settings patch);
  import/export paths come from trusted native dialogs, never renderer text
- Privileged DNS changes still elevate via the standard Windows UAC prompt
  inside the C++ backend (`NetworkManager` → `--apply-dns` helper)

## Design system

Tokens live in `renderer/src/theme.css` (`@theme`): `void/abyss/surface/
raised/overlay`, `ink/ink-dim/ink-faint`, one cyan `accent`, `mint/amber/rose/
sky` status tones, card/pop/glow shadows, `fade-up/scale-in/toast-in` motion.
Dark-first; light theme follows the `themeMode` setting. Icons: `lucide-react`
only. Animation: `framer-motion` with short spring/ease curves.

Dynamic interaction patterns are ported from the OffKnife `react-components`
library (`C:\Users\Junkie\Documents\OffKnife\react-components`):

- `dashboard-sidebar` → collapsible animated rail, hover-reveal shortcut
  hints, live count badges on Profiles/Network, animated active indicator
- `status-badge` → tinted status pills (`Badge` + `StatusBadge` usage)
- `stats-cards-with-circular-progress` → `StatCard` (label + delta + rolling
  `AnimatedNumber` + footer action, staggered spring entrance) and the
  `LatencyGauge` ring on the Dashboard, plus staggered benchmark bars

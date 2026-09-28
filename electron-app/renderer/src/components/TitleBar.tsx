// Custom frameless window bar: drag region + minimize / maximize-restore /
// close with hover states, Alt+Space-safe, keyboard accessible.

import { Copy, Minus, Square, X } from "lucide-react";
import { useEffect, useState } from "react";
import logoUrl from "../assets/dancy.png";
import { cn } from "../utils";

function win() {
  return (window as unknown as { dnsApi?: Record<string, (...a: never[]) => Promise<unknown>> }).dnsApi;
}

export function TitleBar({ onRequestClose }: { onRequestClose: () => void }) {
  const [maximized, setMaximized] = useState(false);

  useEffect(() => {
    const api = win();
    if (!api?.["isMaximized"]) return;
    (api["isMaximized"]() as Promise<{ maximized: boolean }>)
      .then((r) => setMaximized(!!r?.maximized))
      .catch(() => undefined);
    const off = (api["onMaxChanged"] as unknown as (cb: (m: boolean) => void) => () => void)?.(
      (m) => setMaximized(m),
    );
    return () => off?.();
  }, []);

  const min = () => void win()?.["minimizeWindow"]?.();
  const toggle = () => {
    const api = win();
    if (!api?.["toggleMaximize"]) return;
    (api["toggleMaximize"]() as Promise<{ maximized: boolean }>)
      .then((r) => setMaximized(!!r?.maximized))
      .catch(() => undefined);
  };

  const btn =
    "titlebar-no-drag inline-flex h-8 w-11 items-center justify-center text-ink-dim transition-colors hover:bg-hover hover:text-ink";

  return (
    <div className="titlebar-drag flex h-9 shrink-0 items-center gap-2 border-b border-line-soft bg-abyss/95 pr-0 pl-3 select-none">
      <img src={logoUrl} alt="Dancy" className="h-5 w-5 rounded-md object-cover" draggable={false} />
      <span className="text-[12px] font-medium text-ink-dim">Dancy</span>
      <div className="flex-1" />
      <div className="flex items-stretch">
        <button className={btn} title="Minimize" aria-label="Minimize window" onClick={min} tabIndex={-1}>
          <Minus size={15} />
        </button>
        <button
          className={btn}
          title={maximized ? "Restore" : "Maximize"}
          aria-label={maximized ? "Restore window" : "Maximize window"}
          onClick={toggle}
          tabIndex={-1}
        >
          {maximized ? <Copy size={13} /> : <Square size={13} />}
        </button>
        <button
          className={cn(btn, "w-12 hover:bg-rose hover:text-white")}
          title="Close"
          aria-label="Close window"
          onClick={onRequestClose}
          tabIndex={-1}
        >
          <X size={16} />
        </button>
      </div>
    </div>
  );
}

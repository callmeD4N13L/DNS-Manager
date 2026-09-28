// Dynamic stat components ported from OffKnife react-components:
// - `stats-cards-with-circular-progress` (Stats05: label + delta + big value
//   + footer action) → StatCard with spring entrance, hover lift and an
//   animated rolling number.
// - circular progress ring → LatencyGauge (SVG ring, animated sweep).

import { animate, motion } from "framer-motion";
import { useEffect, useRef, useState, type ReactNode } from "react";
import { cn } from "../utils";

export function AnimatedNumber({
  value,
  format = (v: number) => String(Math.round(v)),
  className,
}: {
  value: number;
  format?: (v: number) => string;
  className?: string;
}) {
  const [display, setDisplay] = useState(value);
  const prev = useRef(value);

  useEffect(() => {
    if (prev.current === value) return;
    const controls = animate(prev.current, value, {
      duration: 0.6,
      ease: [0.22, 1, 0.36, 1],
      onUpdate: (v) => setDisplay(v),
    });
    prev.current = value;
    return () => controls.stop();
  }, [value]);

  return <span className={className}>{format(display)}</span>;
}

export function StatCard({
  label,
  hint,
  value,
  footer,
  children,
  index = 0,
}: {
  label: string;
  hint?: ReactNode;
  value: ReactNode;
  footer?: ReactNode;
  children?: ReactNode;
  index?: number;
}) {
  return (
    <motion.section
      initial={{ opacity: 0, y: 14, scale: 0.98 }}
      animate={{ opacity: 1, y: 0, scale: 1 }}
      transition={{ delay: 0.05 * index, type: "spring", stiffness: 320, damping: 28 }}
      whileHover={{ y: -2 }}
      className="flex flex-col rounded-2xl border border-line bg-surface shadow-card"
    >
      <div className="flex items-start justify-between gap-2 px-5 pt-4">
        <span className="truncate text-[12px] font-medium text-ink-dim">{label}</span>
        {hint && <span className="shrink-0 text-[12px] font-medium">{hint}</span>}
      </div>
      <div className="px-5 pt-1 text-[22px] font-semibold tracking-tight text-ink">{value}</div>
      {children && <div className="px-5 pt-2">{children}</div>}
      {footer && (
        <div className="mt-auto flex justify-end border-t border-line-soft px-5 py-2.5">
          {footer}
        </div>
      )}
    </motion.section>
  );
}

const RING = 26;
const CIRC = 2 * Math.PI * RING;

export function LatencyGauge({
  latencyMs,
  size = 64,
  stroke = 6,
}: {
  latencyMs: number;
  size?: number;
  stroke?: number;
}) {
  // 0 ms → full ring, 500+ ms → empty; sweeps with a spring.
  const frac = latencyMs < 0 ? 0 : Math.max(0, 1 - latencyMs / 500);
  const tone =
    latencyMs < 0 ? "text-ink-faint" : latencyMs < 100 ? "text-mint" : latencyMs < 300 ? "text-amber" : "text-rose";
  const strokeColor =
    latencyMs < 0 ? "var(--color-ink-faint)" : latencyMs < 100 ? "var(--color-mint)" : latencyMs < 300 ? "var(--color-amber)" : "var(--color-rose)";

  return (
    <div className="relative shrink-0" style={{ width: size, height: size }} role="img" aria-label={`Latency ${latencyMs} milliseconds`}>
      <svg width={size} height={size} className="-rotate-90">
        <circle cx={size / 2} cy={size / 2} r={RING} fill="none" stroke="var(--color-line)" strokeWidth={stroke} />
        <motion.circle
          cx={size / 2}
          cy={size / 2}
          r={RING}
          fill="none"
          stroke={strokeColor}
          strokeWidth={stroke}
          strokeLinecap="round"
          strokeDasharray={CIRC}
          initial={false}
          animate={{ strokeDashoffset: CIRC * (1 - frac) }}
          transition={{ type: "spring", stiffness: 90, damping: 20 }}
          style={{ filter: "drop-shadow(0 0 6px rgba(34,211,238,0.35))" }}
        />
      </svg>
      <div className={cn("absolute inset-0 flex flex-col items-center justify-center leading-none", tone)}>
        <AnimatedNumber
          value={latencyMs < 0 ? 0 : latencyMs}
          format={(v) => (latencyMs < 0 ? "—" : String(Math.round(v)))}
          className="font-mono text-[15px] font-bold"
        />
        <span className="font-mono text-[9px] opacity-70">ms</span>
      </div>
    </div>
  );
}

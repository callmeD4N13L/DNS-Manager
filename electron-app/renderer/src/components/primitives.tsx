// Design-system primitives: one consistent visual language for the app.

import { motion } from "framer-motion";
import { Loader2 } from "lucide-react";
import type { ButtonHTMLAttributes, InputHTMLAttributes, ReactNode } from "react";
import { cn } from "../utils";

/* --- layout --- */

export function Card({
  title,
  subtitle,
  action,
  children,
  className,
  glow = false,
}: {
  title?: string;
  subtitle?: string;
  action?: ReactNode;
  children: ReactNode;
  className?: string;
  glow?: boolean;
}) {
  return (
    <section
      className={cn(
        "rounded-2xl border border-line bg-surface shadow-card",
        glow && "border-accent/25 shadow-glow",
        className,
      )}
    >
      {(title || action) && (
        <header className="flex items-start justify-between gap-4 px-5 pt-4 pb-1">
          <div className="min-w-0">
            {title && (
              <h2 className="text-[14px] font-semibold tracking-tight text-ink">{title}</h2>
            )}
            {subtitle && <p className="mt-0.5 text-[12px] text-ink-dim">{subtitle}</p>}
          </div>
          {action && <div className="flex shrink-0 items-center gap-2">{action}</div>}
        </header>
      )}
      <div className="px-5 py-4">{children}</div>
    </section>
  );
}

export function PageHeader({
  title,
  subtitle,
  action,
}: {
  title: string;
  subtitle: string;
  action?: ReactNode;
}) {
  return (
    <div className="flex flex-wrap items-end justify-between gap-3">
      <div>
        <h1 className="text-[20px] font-semibold tracking-tight text-ink">{title}</h1>
        <p className="mt-0.5 text-[12.5px] text-ink-dim">{subtitle}</p>
      </div>
      {action && <div className="flex items-center gap-2">{action}</div>}
    </div>
  );
}

/* --- buttons --- */

type BtnProps = ButtonHTMLAttributes<HTMLButtonElement> & {
  icon?: ReactNode;
  loading?: boolean;
};

const btnBase =
  "inline-flex h-9 items-center justify-center gap-2 rounded-lg px-3.5 text-[13px] font-medium whitespace-nowrap outline-none select-none disabled:cursor-not-allowed disabled:opacity-45";

export function PrimaryButton({ icon, loading, children, className, disabled, ...rest }: BtnProps) {
  return (
    <button
      className={cn(
        btnBase,
        "bg-accent text-accent-ink shadow-[0_0_18px_-6px_rgba(34,211,238,0.7)] hover:bg-accent-soft hover:shadow-[0_0_22px_-4px_rgba(34,211,238,0.8)]",
        className,
      )}
      disabled={disabled || loading}
      {...rest}
    >
      {loading ? <Loader2 size={15} className="animate-spin" /> : icon}
      {children}
    </button>
  );
}

export function SecondaryButton({ icon, loading, children, className, disabled, ...rest }: BtnProps) {
  return (
    <button
      className={cn(
        btnBase,
        "border border-line bg-raised text-ink hover:border-accent/40 hover:bg-hover",
        className,
      )}
      disabled={disabled || loading}
      {...rest}
    >
      {loading ? <Loader2 size={15} className="animate-spin" /> : icon}
      {children}
    </button>
  );
}

export function DangerButton({ icon, loading, children, className, disabled, ...rest }: BtnProps) {
  return (
    <button
      className={cn(btnBase, "bg-rose/15 text-rose hover:bg-rose/25", className)}
      disabled={disabled || loading}
      {...rest}
    >
      {loading ? <Loader2 size={15} className="animate-spin" /> : icon}
      {children}
    </button>
  );
}

export function IconButton({
  label,
  children,
  active = false,
  className,
  ...rest
}: ButtonHTMLAttributes<HTMLButtonElement> & {
  label: string;
  active?: boolean;
}) {
  return (
    <button
      title={label}
      aria-label={label}
      className={cn(
        "inline-flex h-8 w-8 items-center justify-center rounded-lg border border-transparent text-ink-dim hover:border-line hover:bg-hover hover:text-ink",
        active && "border-accent/30 bg-accent-wash text-accent",
        className,
      )}
      {...rest}
    >
      {children}
    </button>
  );
}

/* --- badges / dots --- */

export function Dot({ color }: { color: "mint" | "rose" | "amber" | "accent" | "faint" }) {
  const map = {
    mint: "bg-mint shadow-[0_0_8px_rgba(52,211,153,0.9)]",
    rose: "bg-rose shadow-[0_0_8px_rgba(251,113,133,0.9)]",
    amber: "bg-amber shadow-[0_0_8px_rgba(251,191,36,0.9)]",
    accent: "bg-accent shadow-[0_0_8px_rgba(34,211,238,0.9)]",
    faint: "bg-ink-faint",
  } as const;
  return <span className={cn("inline-block h-2 w-2 shrink-0 rounded-full", map[color])} />;
}

export function Badge({
  tone = "neutral",
  children,
}: {
  tone?: "neutral" | "mint" | "amber" | "rose" | "accent" | "sky";
  children: ReactNode;
}) {
  const tones = {
    neutral: "border-line bg-overlay text-ink-dim",
    mint: "border-mint/25 bg-mint-wash text-mint",
    amber: "border-amber/25 bg-amber-wash text-amber",
    rose: "border-rose/25 bg-rose-wash text-rose",
    accent: "border-accent/25 bg-accent-wash text-accent-soft",
    sky: "border-sky/25 bg-sky-wash text-sky",
  } as const;
  return (
    <span
      className={cn(
        "inline-flex items-center gap-1.5 rounded-full border px-2.5 py-0.5 text-[11.5px] font-medium whitespace-nowrap",
        tones[tone],
      )}
    >
      {children}
    </span>
  );
}

/* --- forms --- */

export function Field({
  label,
  hint,
  error,
  children,
  mono = false,
}: {
  label: string;
  hint?: string;
  error?: string;
  children: ReactNode;
  mono?: boolean;
}) {
  return (
    <label className="block">
      <span className="mb-1.5 block text-[12px] font-medium text-ink-dim">{label}</span>
      <div className={mono ? "font-mono" : undefined}>{children}</div>
      {error ? (
        <span className="mt-1 block text-[11.5px] text-rose">{error}</span>
      ) : hint ? (
        <span className="mt-1 block text-[11.5px] text-ink-faint">{hint}</span>
      ) : null}
    </label>
  );
}

export function TextInput({
  className,
  ...rest
}: InputHTMLAttributes<HTMLInputElement>) {
  return (
    <input
      className={cn(
        "h-9 w-full rounded-lg border border-line bg-abyss px-3 text-[13px] text-ink placeholder:text-ink-faint hover:border-ink-faint/60 focus:border-accent/60 focus:bg-surface focus:ring-2 focus:ring-accent/20 focus:outline-none",
        className,
      )}
      {...rest}
    />
  );
}

export function Toggle({
  checked,
  onChange,
  label,
}: {
  checked: boolean;
  onChange: (v: boolean) => void;
  label: string;
}) {
  return (
    <button
      role="switch"
      aria-checked={checked}
      aria-label={label}
      onClick={() => onChange(!checked)}
      className={cn(
        "relative h-6 w-11 shrink-0 rounded-full border transition-colors duration-150",
        checked ? "border-accent/50 bg-accent/90" : "border-line bg-overlay",
      )}
    >
      <motion.span
        layout
        transition={{ type: "spring", stiffness: 600, damping: 32 }}
        className={cn(
          "absolute top-1/2 h-4.5 w-4.5 -translate-y-1/2 rounded-full shadow",
          checked ? "right-1 bg-accent-ink" : "left-1 bg-ink-dim",
        )}
        style={{ width: 18, height: 18 }}
      />
    </button>
  );
}

export function SettingRow({
  label,
  description,
  children,
}: {
  label: string;
  description: string;
  children: ReactNode;
}) {
  return (
    <div className="flex items-center justify-between gap-6 py-1">
      <div className="min-w-0">
        <div className="text-[13px] font-medium text-ink">{label}</div>
        <div className="mt-0.5 text-[12px] text-ink-dim">{description}</div>
      </div>
      <div className="flex shrink-0 items-center gap-2">{children}</div>
    </div>
  );
}

/* --- modal --- */

export function Modal({
  title,
  description,
  children,
  footer,
  onClose,
  wide = false,
}: {
  title: string;
  description?: string;
  children: ReactNode;
  footer: ReactNode;
  onClose: () => void;
  wide?: boolean;
}) {
  return (
    <motion.div
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      exit={{ opacity: 0 }}
      transition={{ duration: 0.16 }}
      className="fixed inset-0 z-50 flex items-center justify-center bg-void/70 p-4 backdrop-blur-[3px]"
      onMouseDown={(e) => {
        if (e.target === e.currentTarget) onClose();
      }}
    >
      <motion.div
        role="dialog"
        aria-modal="true"
        aria-label={title}
        initial={{ opacity: 0, scale: 0.96, y: 10 }}
        animate={{ opacity: 1, scale: 1, y: 0 }}
        exit={{ opacity: 0, scale: 0.97, y: 6 }}
        transition={{ type: "spring", stiffness: 480, damping: 34 }}
        className={cn(
          "w-full rounded-2xl border border-line bg-surface shadow-pop",
          wide ? "max-w-xl" : "max-w-md",
        )}
      >
        <div className="px-5 pt-5 pb-1">
          <h2 className="text-[15px] font-semibold tracking-tight text-ink">{title}</h2>
          {description && <p className="mt-1 text-[12.5px] text-ink-dim">{description}</p>}
        </div>
        <div className="px-5 py-3">{children}</div>
        <div className="flex justify-end gap-2 rounded-b-2xl border-t border-line-soft bg-raised/60 px-5 py-3.5">
          {footer}
        </div>
      </motion.div>
    </motion.div>
  );
}

/* --- states --- */

export function EmptyState({
  icon,
  title,
  hint,
  action,
}: {
  icon: ReactNode;
  title: string;
  hint: string;
  action?: ReactNode;
}) {
  return (
    <div className="flex flex-col items-center gap-1.5 py-10 text-center">
      <div className="mb-1 flex h-11 w-11 items-center justify-center rounded-xl border border-line bg-raised text-ink-faint">
        {icon}
      </div>
      <div className="text-[13.5px] font-medium text-ink-dim">{title}</div>
      <div className="text-[12px] text-ink-faint">{hint}</div>
      {action && <div className="mt-3">{action}</div>}
    </div>
  );
}

export function Skeleton({ className }: { className?: string }) {
  return <div className={cn("animate-pulse rounded-lg bg-overlay", className)} />;
}

export function Spinner({ label }: { label?: string }) {
  return (
    <span className="inline-flex items-center gap-2 text-[12.5px] text-ink-dim">
      <Loader2 size={14} className="animate-spin text-accent" />
      {label}
    </span>
  );
}

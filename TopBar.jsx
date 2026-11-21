import { statusMeta } from "../lib/format";

export default function TopBar({ status, onlineCount, lastPacket }) {
  const meta = statusMeta(status);
  return (
    <header className="border-b border-white/10 bg-[#10181d]">
      <div className="mx-auto flex max-w-6xl flex-col gap-3 px-4 py-4 sm:flex-row sm:items-center sm:justify-between">
        <div>
          <p className="text-xs font-semibold uppercase tracking-[0.18em] text-teal-300">Open-pit model</p>
          <h1 className="text-2xl font-semibold text-white">Slope Guard 360</h1>
          <p className="text-sm text-amber-100/90">Monitoring prototype, not a mine-safety system</p>
        </div>
        <div className="flex flex-wrap items-center gap-2 text-sm">
          <span className={`rounded-full px-3 py-1 font-medium ring-1 ${meta.chip}`}>{meta.label}</span>
          <span className="rounded-full bg-white/5 px-3 py-1 text-slate-200 ring-1 ring-white/10">
            {onlineCount} {onlineCount === 1 ? "node" : "nodes"} online
          </span>
          <span className="rounded-full bg-white/5 px-3 py-1 text-slate-300 ring-1 ring-white/10">
            Last packet {lastPacket}
          </span>
        </div>
      </div>
    </header>
  );
}

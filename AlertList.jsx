import { formatTime, reasonLabel, statusMeta } from "../lib/format";

export default function AlertList({ alerts }) {
  return (
    <section className="rounded-2xl border border-white/10 bg-[#121a20] p-4">
      <h2 className="text-sm font-semibold uppercase tracking-wide text-slate-300">Alerts</h2>
      <p className="mb-3 text-sm text-slate-400">A new row is stored only when a node’s status or reason changes.</p>
      {alerts.length === 0 ? (
        <p className="py-8 text-sm text-slate-400">No alerts yet.</p>
      ) : (
        <ul className="max-h-80 space-y-2 overflow-auto">
          {alerts.map((alert) => {
            const meta = statusMeta(alert.level);
            return (
              <li key={alert.id} className="flex flex-wrap items-center justify-between gap-2 rounded-lg bg-black/20 px-3 py-2 text-sm">
                <span className="text-slate-300">{formatTime(alert.ts)}</span>
                <span className="text-slate-100">{alert.nodeId === "node02" ? "Node 2" : "Node 1"}</span>
                <span className="text-slate-200">{reasonLabel(alert.parameter)}</span>
                <span className={`rounded-full px-2 py-0.5 ring-1 ${meta.chip}`}>{meta.label}</span>
              </li>
            );
          })}
        </ul>
      )}
    </section>
  );
}

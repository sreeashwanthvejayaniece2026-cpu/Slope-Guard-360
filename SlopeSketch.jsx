import { statusMeta } from "../lib/format";

function Marker({ x, y, bench, status }) {
  const meta = statusMeta(status);
  return (
    <g>
      <circle cx={x} cy={y} r="16" fill={meta.dot} stroke="#0b1014" strokeWidth="3" />
      <text x={x} y={y + 5} textAnchor="middle" fontSize="13" fontWeight="700" fill="#0b1014">
        {bench}
      </text>
      <text x={x} y={y + 36} textAnchor="middle" fontSize="13" fill="#e8eef2">
        {meta.label}
      </text>
    </g>
  );
}

export default function SlopeSketch({ statusA, statusB }) {
  return (
    <section className="rounded-2xl border border-white/10 bg-[#121a20] p-4">
      <h2 className="text-sm font-semibold uppercase tracking-wide text-slate-300">Model slope</h2>
      <p className="mb-3 text-sm text-slate-400">Two benches. Colour is repeated as a word so the state is clear without relying on colour alone.</p>
      <svg viewBox="0 0 640 320" className="h-auto w-full" role="img" aria-label="Slope cross-section with benches A and B">
        <rect x="0" y="0" width="640" height="320" rx="12" fill="#0d1419" />
        <path d="M40 250 L160 250 L160 190 L300 190 L300 130 L470 130 L470 70 L600 70 L600 270 L40 270 Z" fill="#24312a" stroke="#3d5a45" strokeWidth="3" />
        <text x="52" y="40" fill="#94a3b8" fontSize="13">Cross-section of the model pit</text>
        <Marker x={210} y={168} bench="A" status={statusA} />
        <Marker x={520} y={48} bench="B" status={statusB} />
      </svg>
    </section>
  );
}

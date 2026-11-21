import { CartesianGrid, Legend, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from "recharts";
import { historyRows, historyToCsv, mergeSeries } from "../lib/format";

function ChartCard({ title, unit, data, domain }) {
  return (
    <div className="h-72 rounded-2xl border border-white/10 bg-[#121a20] p-4">
      <h3 className="mb-2 text-sm font-semibold text-slate-200">{title}</h3>
      {data.length === 0 ? (
        <p className="pt-16 text-center text-sm text-slate-400">No readings yet. Power on a node.</p>
      ) : (
        <ResponsiveContainer width="100%" height="90%">
          <LineChart data={data}>
            <CartesianGrid stroke="#24303a" />
            <XAxis dataKey="label" minTickGap={24} stroke="#94a3b8" fontSize={12} />
            <YAxis unit={unit} domain={domain} stroke="#94a3b8" fontSize={12} />
            <Tooltip />
            <Legend />
            <Line type="monotone" dataKey="node01" name="Node 1" stroke="#5eead4" dot={false} connectNulls strokeWidth={2} />
            <Line type="monotone" dataKey="node02" name="Node 2" stroke="#fbbf24" dot={false} connectNulls strokeWidth={2} />
          </LineChart>
        </ResponsiveContainer>
      )}
    </div>
  );
}

export default function TrendCharts({ history }) {
  const node01 = historyRows(history?.node01);
  const node02 = historyRows(history?.node02);
  const tilt = mergeSeries(node01, node02, "tilt");
  const moisture = mergeSeries(node01, node02, "moisture");

  function download() {
    const csv = historyToCsv(history);
    const blob = new Blob([csv], { type: "text/csv;charset=utf-8" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = "slope-guard-history.csv";
    link.click();
    URL.revokeObjectURL(url);
  }

  return (
    <section className="space-y-3">
      <div className="flex flex-wrap items-center justify-between gap-2">
        <h2 className="text-sm font-semibold uppercase tracking-wide text-slate-300">History</h2>
        <button type="button" onClick={download} className="rounded-lg bg-white/10 px-3 py-2 text-sm text-white hover:bg-white/15">
          Download CSV
        </button>
      </div>
      <div className="grid gap-4 lg:grid-cols-2">
        <ChartCard title="Tilt change (degrees)" unit="°" data={tilt} domain={[0, "auto"]} />
        <ChartCard title="Relative moisture (percent)" unit="%" data={moisture} domain={[0, 100]} />
      </div>
    </section>
  );
}

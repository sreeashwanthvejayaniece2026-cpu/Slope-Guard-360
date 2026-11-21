import AlertList from "./components/AlertList";
import NodeCard from "./components/NodeCard";
import SlopeSketch from "./components/SlopeSketch";
import ThresholdPanel from "./components/ThresholdPanel";
import TopBar from "./components/TopBar";
import TrendCharts from "./components/TrendCharts";
import { useSlopeData } from "./hooks/useSlopeData";
import { NODES, displayStatus, formatTime, isOffline } from "./lib/format";

function SetupScreen() {
  return (
    <main className="mx-auto max-w-xl px-4 py-16 text-slate-100">
      <p className="text-xs font-semibold uppercase tracking-[0.18em] text-teal-300">Slope Guard 360</p>
      <h1 className="mt-2 text-3xl font-semibold">Add the Firebase keys</h1>
      <p className="mt-3 text-slate-300">
        Copy <code>dashboard/.env.example</code> to <code>dashboard/.env</code>, paste the five
        <code> VITE_FIREBASE_ </code> values from Firebase project settings, then stop the site and run
        <code> npm run dev </code> again.
      </p>
      <p className="mt-4 text-sm text-amber-100">Monitoring prototype, not a mine-safety system.</p>
    </main>
  );
}

export default function App() {
  const data = useSlopeData();

  if (!data.ready) return <SetupScreen />;

  const offlineSeconds = Number(data.thresholds.offlineSeconds) || 15;
  const statusA = displayStatus(data.nodes.node01, data.serverNow, offlineSeconds);
  const statusB = displayStatus(data.nodes.node02, data.serverNow, offlineSeconds);
  const onlineCount = [data.nodes.node01, data.nodes.node02].filter(
    (node) => node && !isOffline(node, data.serverNow, offlineSeconds)
  ).length;

  const rank = { offline: 0, normal: 1, caution: 2, warning: 3 };
  let system = "offline";
  if (onlineCount > 0) {
    system = "normal";
    if (rank[statusA] > rank[system] && statusA !== "offline") system = statusA;
    if (rank[statusB] > rank[system] && statusB !== "offline") system = statusB;
  }

  const times = [data.nodes.node01?.ts, data.nodes.node02?.ts].filter((ts) => typeof ts === "number");
  const lastPacket = times.length ? formatTime(Math.max(...times)) : "No packet yet";

  return (
    <div className="min-h-screen bg-[#0b1014]">
      <TopBar status={system} onlineCount={onlineCount} lastPacket={lastPacket} />
      <main className="mx-auto grid max-w-6xl gap-4 px-4 py-4">
        {!data.connected && <p className="text-sm text-slate-300">Connecting to Firebase…</p>}
        {data.error && <p className="rounded-lg bg-red-950/60 px-3 py-2 text-sm text-red-100">{data.error}</p>}
        <div className="grid gap-4 lg:grid-cols-[1.15fr_0.85fr]">
          <SlopeSketch statusA={statusA} statusB={statusB} />
          <div className="grid gap-4">
            {NODES.map((node) => (
              <NodeCard
                key={node.id}
                nodeInfo={node}
                reading={data.nodes[node.id]}
                serverNow={data.serverNow}
                offlineSeconds={offlineSeconds}
                onRebaseline={() => data.rebaseline(node.id)}
              />
            ))}
          </div>
        </div>
        <TrendCharts history={data.history} />
        <div className="grid gap-4 lg:grid-cols-2">
          <AlertList alerts={data.alerts} />
          <ThresholdPanel thresholds={data.thresholds} onSave={data.saveThresholds} />
        </div>
        <p className="pb-6 text-sm text-slate-500">
          Each ESP32 reads its own sensors, decides normal, caution, or warning, and sends that result here.
        </p>
      </main>
    </div>
  );
}

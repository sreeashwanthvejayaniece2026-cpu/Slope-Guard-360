import { displayStatus, formatTime, isOffline, reasonLabel, statusMeta } from "../lib/format";

function Row({ label, value }) {
  return (
    <div className="flex items-baseline justify-between gap-3 border-b border-white/5 py-2 text-sm">
      <span className="text-slate-400">{label}</span>
      <span className="text-right font-medium text-slate-100">{value}</span>
    </div>
  );
}

export default function NodeCard({ nodeInfo, reading, serverNow, offlineSeconds, onRebaseline }) {
  const offline = isOffline(reading, serverNow, offlineSeconds);
  const status = displayStatus(reading, serverNow, offlineSeconds);
  const meta = statusMeta(status);
  const rainText = reading && Object.prototype.hasOwnProperty.call(reading, "rainWet")
    ? (reading.rainWet ? "Wet" : "Dry")
    : "Not fitted";

  return (
    <article className="rounded-2xl border border-white/10 bg-[#121a20] p-4">
      <div className="mb-2 flex items-start justify-between gap-3">
        <div>
          <h2 className="text-lg font-semibold text-white">{nodeInfo.title}</h2>
          <p className="text-sm text-slate-400">{nodeInfo.place}</p>
        </div>
        <span className={`rounded-full px-3 py-1 text-sm font-medium ring-1 ${meta.chip}`}>{meta.label}</span>
      </div>

      {!reading && <p className="py-6 text-sm text-slate-400">No data yet. Power on this ESP32 and wait for the first packet.</p>}

      {reading && (
        <>
          <Row label="Tilt change" value={`${Number(reading.tilt || 0).toFixed(1)}°  (X ${Number(reading.tiltX || 0).toFixed(1)}°, Y ${Number(reading.tiltY || 0).toFixed(1)}°)`} />
          <Row label="Relative moisture" value={`${Number(reading.moisture || 0).toFixed(0)}%`} />
          <Row label="Rain plate" value={rainText} />
          <Row label="Link" value={offline ? "Offline" : "Online"} />
          <Row label="Last seen" value={formatTime(reading.ts)} />
          <Row label="Reason" value={offline ? `${reasonLabel(reading.reason)} (last report)` : reasonLabel(reading.reason)} />
        </>
      )}

      <button
        type="button"
        onClick={onRebaseline}
        className="mt-4 w-full rounded-lg bg-teal-700 px-3 py-2 text-sm font-medium text-white hover:bg-teal-600"
      >
        Set baseline
      </button>
      <p className="mt-2 text-xs text-slate-500">Hold the model still. The node averages the next 5 quiet seconds and treats that pose as zero.</p>
    </article>
  );
}

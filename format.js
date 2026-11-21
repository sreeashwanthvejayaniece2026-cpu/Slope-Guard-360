export const NODES = [
  { id: "node01", bench: "A", title: "Node 1", place: "Bench A" },
  { id: "node02", bench: "B", title: "Node 2", place: "Bench B" },
];

export const DEFAULT_THRESHOLDS = {
  tiltCaution: 5,
  tiltWarning: 10,
  moistureCaution: 60,
  moistureWarning: 80,
  offlineSeconds: 15,
};

export function statusMeta(status) {
  if (status === "warning") {
    return { label: "Warning", dot: "#f87171", chip: "bg-red-500/15 text-red-200 ring-red-400/40" };
  }
  if (status === "caution") {
    return { label: "Caution", dot: "#fbbf24", chip: "bg-amber-400/15 text-amber-100 ring-amber-300/40" };
  }
  if (status === "offline") {
    return { label: "Offline", dot: "#94a3b8", chip: "bg-slate-500/20 text-slate-200 ring-slate-400/30" };
  }
  return { label: "Normal", dot: "#34d399", chip: "bg-emerald-400/15 text-emerald-100 ring-emerald-300/40" };
}

export function reasonLabel(reason) {
  if (reason === "tilt") return "Tilt";
  if (reason === "moisture") return "Moisture";
  if (reason === "rain") return "Rain plate";
  if (reason === "moisture+rain") return "Moisture and rain";
  return "None";
}

export function formatTime(ts) {
  if (typeof ts !== "number") return "No packet yet";
  return new Date(ts).toLocaleString(undefined, {
    day: "2-digit",
    month: "short",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
  });
}

export function isOffline(node, serverNow, offlineSeconds) {
  if (!node || typeof node.ts !== "number") return true;
  return serverNow - node.ts > offlineSeconds * 1000;
}

export function displayStatus(node, serverNow, offlineSeconds) {
  if (isOffline(node, serverNow, offlineSeconds)) return "offline";
  if (node.status === "warning" || node.status === "caution" || node.status === "normal") {
    return node.status;
  }
  return "normal";
}

export function historyRows(history) {
  if (!history) return [];
  return Object.values(history)
    .filter((row) => row && typeof row.ts === "number")
    .sort((a, b) => a.ts - b.ts);
}

export function mergeSeries(left, right, field) {
  const bySecond = new Map();
  for (const row of left) {
    const key = Math.round(row.ts / 1000);
    bySecond.set(key, {
      ts: row.ts,
      label: new Date(row.ts).toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit" }),
      node01: Number(row[field]) || 0,
    });
  }
  for (const row of right) {
    const key = Math.round(row.ts / 1000);
    const existing = bySecond.get(key) || {
      ts: row.ts,
      label: new Date(row.ts).toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit", second: "2-digit" }),
    };
    existing.node02 = Number(row[field]) || 0;
    bySecond.set(key, existing);
  }
  return Array.from(bySecond.values()).sort((a, b) => a.ts - b.ts);
}

function csvCell(value) {
  const text = value == null ? "" : String(value);
  if (/[",\n]/.test(text)) return `"${text.replace(/"/g, '""')}"`;
  return text;
}

export function historyToCsv(historyByNode) {
  const header = ["nodeId", "ts", "time", "tiltX", "tiltY", "tilt", "moisture", "rawMoisture", "rainWet", "rawRain", "status", "reason", "rssi"];
  const lines = [header.join(",")];
  for (const nodeId of ["node01", "node02"]) {
    for (const row of historyRows(historyByNode?.[nodeId])) {
      lines.push(
        [
          nodeId,
          row.ts,
          formatTime(row.ts),
          row.tiltX,
          row.tiltY,
          row.tilt,
          row.moisture,
          row.rawMoisture,
          row.rainWet,
          row.rawRain,
          row.status,
          row.reason,
          row.rssi,
        ].map(csvCell).join(",")
      );
    }
  }
  return lines.join("\n");
}

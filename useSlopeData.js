import { useEffect, useState } from "react";
import { get, increment, onValue, ref, set, update } from "firebase/database";
import { configIsReady, firebaseConfigFromEnv, openDatabase } from "../firebase";
import { DEFAULT_THRESHOLDS } from "../lib/format";

export function useSlopeData() {
  const config = firebaseConfigFromEnv();
  const ready = configIsReady(config);
  const [connected, setConnected] = useState(false);
  const [error, setError] = useState("");
  const [nodes, setNodes] = useState({});
  const [history, setHistory] = useState({});
  const [alerts, setAlerts] = useState([]);
  const [thresholds, setThresholds] = useState(DEFAULT_THRESHOLDS);
  const [offset, setOffset] = useState(0);
  const [tick, setTick] = useState(0);
  const [db, setDb] = useState(null);

  useEffect(() => {
    const id = setInterval(() => setTick((n) => n + 1), 1000);
    return () => clearInterval(id);
  }, []);

  useEffect(() => {
    if (!ready) return undefined;
    const database = openDatabase(config);
    setDb(database);

    const watch = (path, onData) =>
      onValue(ref(database, path), (snap) => {
        setConnected(true);
        setError("");
        onData(snap.val());
      }, (err) => setError(err.message || "Firebase read failed."));

    const stops = [
      watch("nodes", (value) => setNodes(value || {})),
      watch("history", (value) => setHistory(value || {})),
      watch("alerts", (value) => {
        const rows = Object.entries(value || {})
          .map(([id, row]) => ({ id, ...row }))
          .filter((row) => typeof row.ts === "number")
          .sort((a, b) => b.ts - a.ts)
          .slice(0, 50);
        setAlerts(rows);
      }),
      watch("thresholds", (value) => {
        if (value) setThresholds({ ...DEFAULT_THRESHOLDS, ...value });
      }),
      // Firebase server clock minus this laptop's clock. Offline checks use it
      // so a dead node still flips to offline when no new packets arrive.
      watch(".info/serverTimeOffset", (value) => setOffset(typeof value === "number" ? value : 0)),
    ];

    get(ref(database, "thresholds")).then((snap) => {
      if (!snap.exists()) {
        set(ref(database, "thresholds"), DEFAULT_THRESHOLDS).catch((err) => {
          setError(err.message || "Could not save the default thresholds.");
        });
      }
    });

    return () => stops.forEach((stop) => stop());
    // The config object is rebuilt each render. ready is the real gate.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [ready]);

  async function saveThresholds(next) {
    if (!db) return;
    await update(ref(db, "thresholds"), {
      tiltCaution: Number(next.tiltCaution),
      tiltWarning: Number(next.tiltWarning),
      moistureCaution: Number(next.moistureCaution),
      moistureWarning: Number(next.moistureWarning),
      offlineSeconds: Number(next.offlineSeconds),
    });
  }

  async function rebaseline(nodeId) {
    if (!db) return;
    await update(ref(db, `commands/${nodeId}`), { rebaseline: increment(1) });
  }

  return {
    ready,
    connected,
    error,
    nodes,
    history,
    alerts,
    thresholds,
    serverNow: Date.now() + offset + tick * 0,
    saveThresholds,
    rebaseline,
  };
}

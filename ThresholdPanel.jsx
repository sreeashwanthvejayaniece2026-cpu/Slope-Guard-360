import { useEffect, useState } from "react";

const FIELDS = [
  ["tiltCaution", "Tilt caution (degrees)"],
  ["tiltWarning", "Tilt warning (degrees)"],
  ["moistureCaution", "Moisture caution (percent)"],
  ["moistureWarning", "Moisture warning (percent)"],
  ["offlineSeconds", "Offline after (seconds)"],
];

export default function ThresholdPanel({ thresholds, onSave }) {
  const [draft, setDraft] = useState(thresholds);
  const [dirty, setDirty] = useState(false);
  const [message, setMessage] = useState("");

  useEffect(() => {
    if (!dirty) setDraft(thresholds);
  }, [thresholds, dirty]);

  function updateField(key, value) {
    setDirty(true);
    setMessage("");
    setDraft((prev) => ({ ...prev, [key]: value }));
  }

  async function save(event) {
    event.preventDefault();
    const next = {
      tiltCaution: Number(draft.tiltCaution),
      tiltWarning: Number(draft.tiltWarning),
      moistureCaution: Number(draft.moistureCaution),
      moistureWarning: Number(draft.moistureWarning),
      offlineSeconds: Number(draft.offlineSeconds),
    };
    if (!(next.tiltWarning > next.tiltCaution) || !(next.moistureWarning > next.moistureCaution)) {
      setMessage("Each warning limit has to be higher than its caution limit.");
      return;
    }
    if (next.offlineSeconds < 5 || next.offlineSeconds > 300) {
      setMessage("Offline time must be between 5 and 300 seconds.");
      return;
    }
    try {
      await onSave(next);
      setDirty(false);
      setMessage("Saved. The nodes pick this up within about 10 seconds.");
    } catch (err) {
      setMessage(err.message || "Firebase rejected the save. Check the database rules.");
    }
  }

  return (
    <section className="rounded-2xl border border-white/10 bg-[#121a20] p-4">
      <h2 className="text-sm font-semibold uppercase tracking-wide text-slate-300">Thresholds</h2>
      <p className="mb-3 text-sm text-amber-100/80">Experimental prototype thresholds. Not mine-safety limits.</p>
      <form onSubmit={save} className="space-y-3">
        {FIELDS.map(([key, label]) => (
          <label key={key} className="block text-sm text-slate-300">
            {label}
            <input
              type="number"
              step="0.1"
              value={draft[key]}
              onChange={(event) => updateField(key, event.target.value)}
              className="mt-1 w-full rounded-lg border border-white/10 bg-black/30 px-3 py-2 text-white"
            />
          </label>
        ))}
        <button type="submit" className="w-full rounded-lg bg-white/10 px-3 py-2 font-medium text-white hover:bg-white/15">
          Save thresholds
        </button>
        {message && <p className="text-sm text-slate-300">{message}</p>}
      </form>
    </section>
  );
}

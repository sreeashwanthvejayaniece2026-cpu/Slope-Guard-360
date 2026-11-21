# Firebase setup (free Spark plan)

Do this once. Add all four teammates as owners on the project.

1. Open https://console.firebase.google.com and create a project named `slope-guard-360`. Analytics can stay off.
2. In the left menu open **Build → Realtime Database → Create Database**.
3. Choose the **Singapore** region if it is offered (`asia-southeast1`). That is the closest region in the free list for a demo in India.
4. When it asks for rules, choose **locked mode**. Do not rely on test mode. Test-mode rules expire after about 30 days and the data will silently stop.
5. Open the **Rules** tab. Replace the whole text with the contents of `firebase/database.rules.json` in this repo. Click **Publish**.
6. Copy the database URL. It looks like `https://slope-guard-360-default-rtdb.asia-southeast1.firebasedatabase.app`. That exact host (without `https://`) goes in `firmware/secrets.h` as `FIREBASE_HOST`. The full URL goes in `dashboard/.env` as `VITE_FIREBASE_DATABASE_URL`.
7. Open **Project settings → General → Your apps → Add app → Web**. Register an app named `slope-guard-dashboard`. Copy the config into `dashboard/.env`. You only need the API key, auth domain, database URL, project id, and app id.
8. Open **Project settings → Users and permissions** and invite the other three members as **Owner**.

## What these rules allow

Anyone who knows the database URL can read and write the five paths the project uses: `nodes`, `history`, `alerts`, `thresholds`, and `commands`. Every other path is denied. Writes are checked for the right field types and for numbers inside a sensible range.

## Write this in the report

Public write access is not secure. A stranger with the URL could overwrite readings or thresholds. That is acceptable for a short college demo on a free plan, and it is not acceptable for a real mine. The rules also do not prove who sent a reading. Say that under limitations.

## A week before the viva

Open the Rules tab and confirm the published rules are still the ones from `database.rules.json`. If someone turned on test mode, the database will reject writes after the expiry and both nodes will look offline.

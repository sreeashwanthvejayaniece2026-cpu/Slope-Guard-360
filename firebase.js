import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

export function firebaseConfigFromEnv() {
  return {
    apiKey: import.meta.env.VITE_FIREBASE_API_KEY || "",
    authDomain: import.meta.env.VITE_FIREBASE_AUTH_DOMAIN || "",
    databaseURL: import.meta.env.VITE_FIREBASE_DATABASE_URL || "",
    projectId: import.meta.env.VITE_FIREBASE_PROJECT_ID || "",
    appId: import.meta.env.VITE_FIREBASE_APP_ID || "",
  };
}

export function configIsReady(config) {
  return Boolean(config.apiKey && config.databaseURL && config.projectId && config.appId);
}

export function openDatabase(config) {
  const app = initializeApp(config);
  return getDatabase(app);
}

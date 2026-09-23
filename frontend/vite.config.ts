import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// Dev-server proxy so the frontend can call /api/* and /ws/* without CORS
// hassle while developing locally with `npm run dev`. In production the
// built static files are served separately (nginx) and talk to the backend
// via the VITE_API_URL / VITE_WS_URL env vars baked in at build time.
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    proxy: {
      "/api": {
        target: "http://localhost:8080",
        changeOrigin: true,
      },
      "/ws": {
        target: "ws://localhost:8080",
        ws: true,
      },
    },
  },
});

import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import { resolve } from "node:path";

export default defineConfig({
  plugins: [vue()],
  build: {
    outDir: resolve(__dirname, "../public"),
    emptyOutDir: false,
    assetsDir: "assets",
  },
  server: {
    port: 5173,
    proxy: { "/api": "http://127.0.0.1:3000" },
  },
});

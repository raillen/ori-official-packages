/* @jsxImportSource solid-js */
import { createSignal } from "solid-js";

export default function App() {
  const [count, setCount] = createSignal(0);
  return (
    <main style={{ "font-family": "system-ui", "max-width": "32rem", margin: "2rem auto", padding: "0 1rem" }}>
      <h1>Solid + Ori</h1>
      <p>Official UI preset. Build into <code>public/</code>.</p>
      <button type="button" onClick={() => setCount(count() + 1)}>
        Clicks: {count()}
      </button>
    </main>
  );
}

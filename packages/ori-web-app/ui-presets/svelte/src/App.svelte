<script>
  let count = $state(0);
</script>

<main>
  <h1>Svelte + Ori</h1>
  <p>Official UI preset. Build with <code>npm run build</code> into <code>public/</code>.</p>
  <button onclick={() => count++}>Clicks: {count}</button>
</main>

<style>
  main { font-family: system-ui, sans-serif; max-width: 32rem; margin: 2rem auto; padding: 0 1rem; }
  button { font: inherit; padding: 0.5rem 0.9rem; border-radius: 0.5rem; border: 1px solid #ccc; cursor: pointer; }
</style>

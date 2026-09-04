<script setup>
import { ref } from "vue";
const count = ref(0);
</script>

<template>
  <main>
    <h1>Vue + Ori</h1>
    <p>Official UI preset. Build into <code>public/</code>.</p>
    <button @click="count++">Clicks: {{ count }}</button>
  </main>
</template>

<style scoped>
main { font-family: system-ui, sans-serif; max-width: 32rem; margin: 2rem auto; padding: 0 1rem; }
button { font: inherit; padding: 0.5rem 0.9rem; border-radius: 0.5rem; border: 1px solid #ccc; cursor: pointer; }
</style>

<template>
  <div class="log-panel">
    <div class="log-title">UCI</div>
    <div class="log-lines" ref="listRef">
      <div v-for="(line, i) in log" :key="i" class="log-line">{{ line }}</div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, nextTick } from 'vue'

const props = defineProps({
  log: { type: Array, default: () => [] }
})

const listRef = ref(null)

watch(() => props.log.length, async () => {
  await nextTick()
  if (listRef.value) {
    listRef.value.scrollTop = listRef.value.scrollHeight
  }
})
</script>

<style scoped>
.log-panel {
  background: #1e1c1a;
  border-radius: 4px;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}
.log-title {
  padding: 6px 10px;
  font-weight: 600;
  font-size: 0.75rem;
  color: #666;
  text-transform: uppercase;
  letter-spacing: 0.05em;
  border-bottom: 1px solid #2a2826;
}
.log-lines {
  flex: 1;
  overflow-y: auto;
  padding: 4px 0;
  max-height: 120px;
  font-family: 'SF Mono', 'Fira Code', monospace;
  font-size: 0.7rem;
}
.log-line {
  padding: 1px 10px;
  color: #777;
  white-space: nowrap;
}
.log-line:nth-child(even) { background: rgba(255,255,255,0.02); }
</style>

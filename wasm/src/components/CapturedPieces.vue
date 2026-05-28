<template>
  <div class="captured" :class="{ empty: sorted.length === 0 }">
    <span v-for="(p, i) in sorted" :key="i" class="cp" :class="p.color">{{ p.symbol }}</span>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { PIECES } from '@/utils/board.js'

const props = defineProps({
  pieces: { type: Array, default: () => [] }
})

const ORDER = { K: 0, Q: 1, R: 2, B: 3, N: 4, P: 5 }

const sorted = computed(() => {
  return [...props.pieces]
    .sort((a, b) => ORDER[a.toUpperCase()] - ORDER[b.toUpperCase()])
    .map(p => ({
      symbol: PIECES[p],
      color: p === p.toUpperCase() ? 'white' : 'black'
    }))
})
</script>

<style scoped>
.captured {
  display: flex;
  flex-wrap: wrap;
  gap: 0;
  min-height: 0;
  padding: 0 14px;
}
.captured.empty {
  padding: 0;
}
.cp {
  font-size: 1rem;
  line-height: 1.2;
}
.cp.white { color: #fff; }
.cp.black { color: #bbb; }
</style>

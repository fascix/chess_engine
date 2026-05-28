<template>
  <div class="move-list">
    <div class="move-list-title">Partie</div>
    <div class="move-pairs" ref="listRef">
      <div v-for="(pair, i) in formattedMoves" :key="i" class="move-pair">
        <span class="move-num">{{ pair.num }}.</span>
        <span v-if="pair.white" class="move-w">{{ pair.white }}</span>
        <span v-if="pair.black" class="move-b">{{ pair.black }}</span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed, ref, watch, nextTick } from 'vue'

const props = defineProps({
  moves: { type: Array, default: () => [] },
  humanColor: { type: String, default: 'w' }
})

const listRef = ref(null)

const formattedMoves = computed(() => {
  const result = []
  for (let i = 0; i < props.moves.length; i++) {
    const moveNum = Math.floor((i + 1) / 2) + 1
    if (props.humanColor === 'w') {
      if (i % 2 === 0) {
        result.push({ num: moveNum, white: props.moves[i], black: props.moves[i + 1] || '' })
      }
    } else {
      if (i % 2 === 0) {
        if (i === 0) result.push({ num: moveNum, white: '', black: props.moves[i] })
        else result.push({ num: moveNum, white: props.moves[i] || '', black: props.moves[i + 1] || '' })
      }
    }
  }
  return result
})

watch(() => props.moves.length, async () => {
  await nextTick()
  if (listRef.value) {
    listRef.value.scrollTop = listRef.value.scrollHeight
  }
})
</script>

<style scoped>
.move-list {
  background: #2b2826;
  border-radius: 4px;
  overflow: hidden;
  display: flex;
  flex-direction: column;
  min-height: 0;
}
.move-list-title {
  padding: 10px 14px;
  font-weight: 600;
  font-size: 0.9rem;
  color: #aaa;
  text-transform: uppercase;
  letter-spacing: 0.05em;
  border-bottom: 1px solid #3a3735;
}
.move-pairs {
  flex: 1;
  overflow-y: auto;
  padding: 4px 0;
}
.move-pair {
  display: flex;
  gap: 8px;
  padding: 3px 14px;
  font-size: 0.9rem;
  font-variant-numeric: tabular-nums;
}
.move-pair:nth-child(even) { background: rgba(255,255,255,0.03); }
.move-num { color: #666; min-width: 2em; }
.move-w { color: #ddd; min-width: 6em; }
.move-b { color: #ddd; min-width: 6em; }
</style>

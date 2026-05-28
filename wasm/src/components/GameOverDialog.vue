<template>
  <div class="overlay" @click.self="$emit('dismiss')">
    <div class="dialog">
      <button class="close-btn" @click="$emit('dismiss')">✕</button>
      <div class="result-icon">{{ resultIcon }}</div>
      <div class="result-text">{{ resultText }}</div>
      <button class="btn" @click="$emit('back')">Nouvelle partie</button>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const props = defineProps({
  winner: { type: String, default: null },
  humanColor: { type: String, default: 'w' }
})

defineEmits(['back', 'dismiss'])

const resultIcon = computed(() => {
  if (props.winner === 'draw') return '½–½'
  return '1–0'
})

const resultText = computed(() => {
  if (props.winner === 'draw') return 'Partie nulle'
  if (props.winner === 'white') {
    return props.humanColor === 'w' ? 'Vous gagnez !' : 'Pallas gagne'
  }
  return props.humanColor === 'b' ? 'Vous gagnez !' : 'Pallas gagne'
})
</script>

<style scoped>
.overlay {
  position: fixed;
  inset: 0;
  background: rgba(0,0,0,0.7);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 100;
}
.dialog {
  position: relative;
  background: #2b2826;
  border-radius: 12px;
  padding: 40px 50px;
  text-align: center;
  color: #fff;
  box-shadow: 0 8px 40px rgba(0,0,0,0.5);
}
.close-btn {
  position: absolute;
  top: 10px;
  right: 14px;
  background: none;
  border: none;
  color: #666;
  font-size: 1.2rem;
  cursor: pointer;
  padding: 4px 8px;
  border-radius: 4px;
  transition: color 0.15s, background 0.15s;
}
.close-btn:hover {
  color: #fff;
  background: #3a3735;
}
.result-icon {
  font-size: 3rem;
  font-weight: 700;
  margin-bottom: 10px;
}
.result-text {
  font-size: 1.5rem;
  margin-bottom: 24px;
}
.btn {
  padding: 10px 30px;
  background: #7fa650;
  color: #fff;
  border: none;
  border-radius: 6px;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
}
.btn:hover { background: #8fb75a; }
</style>

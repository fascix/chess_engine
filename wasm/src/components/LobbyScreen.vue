<template>
  <div class="lobby">
    <div class="card">
      <h1 class="title">Pallas</h1>
      <p class="subtitle">Moteur d'échecs</p>

      <div class="section">
        <label class="label">Mode de jeu</label>
        <div class="options">
          <button
            v-for="opt in modes" :key="opt.value"
            class="option"
            :class="{ selected: selectedMode === opt.value }"
            @click="selectedMode = opt.value"
          >{{ opt.label }}</button>
        </div>
      </div>

      <div class="section">
        <label class="label">Vous jouez</label>
        <div class="options">
          <button
            v-for="opt in colors" :key="opt.value"
            class="option"
            :class="{ selected: selectedColor === opt.value }"
            @click="selectedColor = opt.value"
          >{{ opt.label }}</button>
        </div>
      </div>

      <div v-if="selectedMode === 'test'" class="section">
        <label class="label">Profondeur de recherche</label>
        <select v-model="depth" class="select">
          <option v-for="d in [1,2,3,4,5,6,7,8,9,10,12,15,20]" :key="d" :value="d">{{ d }}</option>
        </select>
      </div>

      <button class="play-btn" :disabled="!engineReady" @click="$emit('start', selectedMode, selectedColor)">
        {{ engineReady ? 'Commencer' : 'Chargement…' }}
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

defineProps({
  engineReady: Boolean
})

defineEmits(['start'])

const selectedMode = ref('rapid')
const selectedColor = ref('random')
const depth = ref(10)

const modes = [
  { value: 'blitz', label: 'Blitz 5 min' },
  { value: 'rapid', label: 'Rapide 10 min' },
  { value: 'test', label: 'Test' }
]

const colors = [
  { value: 'white', label: 'Blanc' },
  { value: 'black', label: 'Noir' },
  { value: 'random', label: 'Aléatoire' }
]
</script>

<style scoped>
.lobby {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}
.card {
  background: #2b2826;
  border-radius: 12px;
  padding: 36px 40px;
  width: 100%;
  max-width: 400px;
  text-align: center;
  box-shadow: 0 8px 40px rgba(0,0,0,0.4);
}
.title {
  font-size: 2.2rem;
  font-weight: 800;
  margin: 0 0 2px;
  color: #fff;
}
.subtitle {
  color: #888;
  margin: 0 0 28px;
  font-size: 0.95rem;
}
.section {
  margin-bottom: 20px;
  text-align: left;
}
.label {
  display: block;
  color: #aaa;
  font-size: 0.8rem;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.05em;
  margin-bottom: 8px;
}
.options {
  display: flex;
  gap: 6px;
}
.option {
  flex: 1;
  padding: 8px 0;
  background: #3a3735;
  color: #ccc;
  border: 2px solid transparent;
  border-radius: 6px;
  font-size: 0.85rem;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.15s;
}
.option:hover { background: #4a4745; }
.option.selected {
  background: #5a554f;
  border-color: #7fa650;
  color: #fff;
}
.select {
  width: 100%;
  padding: 8px;
  background: #3a3735;
  color: #ccc;
  border: 1px solid #555;
  border-radius: 6px;
  font-size: 0.9rem;
}
.play-btn {
  margin-top: 8px;
  width: 100%;
  padding: 12px;
  background: #7fa650;
  color: #fff;
  border: none;
  border-radius: 6px;
  font-size: 1.1rem;
  font-weight: 700;
  cursor: pointer;
  transition: background 0.15s;
}
.play-btn:hover:not(:disabled) { background: #8fb75a; }
.play-btn:disabled { opacity: 0.5; cursor: not-allowed; }
</style>

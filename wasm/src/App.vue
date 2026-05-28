<template>
  <div class="app-shell">
    <header class="app-header">
      <button class="home-link" @click="goHome">
        <span class="home-icon">♚</span> Pallas
      </button>
    </header>
    <main class="app-main">
      <LobbyScreen
        v-if="phase === 'lobby'"
        :engine-ready="engine.ready.value"
        @start="onStart"
      />
      <GameScreen
        v-else
        :game="game"
        @back-to-lobby="onBackToLobby"
      />
    </main>
    <footer class="app-footer">
      <span>{{ year }} — tous droits réservés</span>
    </footer>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import LobbyScreen from './components/LobbyScreen.vue'
import GameScreen from './components/GameScreen.vue'
import { useGame } from './composables/useGame.js'

const game = useGame()
const { phase, engine, humanColor, startGame } = game
const year = ref(new Date().getFullYear())

onMounted(() => {
  engine.subscribeStdout(game.handleEngineOutput)
  engine.init()
})

function onStart(mode, color) {
  let finalColor = color
  if (color === 'random') {
    finalColor = Math.random() < 0.5 ? 'w' : 'b'
  } else if (color === 'white') {
    finalColor = 'w'
  } else {
    finalColor = 'b'
  }
  startGame(mode, finalColor)
}

function onBackToLobby() {
  game.backToLobby()
}

function goHome() {
  if (phase.value !== 'lobby') game.backToLobby()
}
</script>

<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  background: #312e2b;
  color: #fff;
  min-height: 100vh;
}

::-webkit-scrollbar { width: 6px; }
::-webkit-scrollbar-track { background: transparent; }
::-webkit-scrollbar-thumb { background: #555; border-radius: 3px; }

.app-shell {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
}

.app-header {
  display: flex;
  align-items: center;
  padding: 8px 20px;
  background: #2b2826;
  border-bottom: 1px solid #3a3735;
}

.home-link {
  background: none;
  border: none;
  color: #aaa;
  font-size: 1rem;
  font-weight: 600;
  cursor: pointer;
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 4px 8px;
  border-radius: 4px;
  transition: color 0.15s, background 0.15s;
}
.home-link:hover {
  color: #fff;
  background: #3a3735;
}

.home-icon {
  font-size: 1.2rem;
}

.app-main {
  flex: 1;
}

.app-footer {
  text-align: center;
  padding: 12px 20px;
  color: #555;
  font-size: 0.8rem;
  border-top: 1px solid #2b2826;
}
</style>

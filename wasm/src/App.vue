<template>
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
</template>

<script setup>
import { onMounted } from 'vue'
import LobbyScreen from './components/LobbyScreen.vue'
import GameScreen from './components/GameScreen.vue'
import { useGame } from './composables/useGame.js'

const game = useGame()
const { phase, engine, humanColor, startGame } = game

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
</style>

<template>
  <div class="game-container">
    <div class="main-col">
      <PlayerBar
        name="Pallas"
        :time-display="humanColor === 'w' ? timer.blackDisplay : timer.whiteDisplay"
        :active="!isHumanTurn() && !isThinking"
        :dot-color="humanColor === 'w' ? 'black' : 'white'"
      />
      <ChessBoard
        :board="board"
        :selected-sq="selectedSq"
        :last-move-sq="lastMoveSq"
        :legal-destinations="legalDestinations"
        :flipped="humanColor === 'b'"
        @square-click="onSquareClick"
      />
      <PlayerBar
        name="Vous"
        :time-display="humanColor === 'w' ? timer.whiteDisplay : timer.blackDisplay"
        :active="!isThinking && isHumanTurn()"
        :dot-color="humanColor"
      />
      <button class="new-game-btn" @click="$emit('backToLobby')">Nouvelle partie</button>
    </div>
    <div class="side-col">
      <MoveList :moves="moveHistory" :human-color="humanColor" />
      <LogPanel :log="uciLog" />
    </div>
    <GameOverDialog
      v-if="phase === 'gameover'"
      :winner="winner"
      :human-color="humanColor"
      @back="$emit('backToLobby')"
    />
  </div>
</template>

<script setup>
import PlayerBar from './PlayerBar.vue'
import ChessBoard from './ChessBoard.vue'
import MoveList from './MoveList.vue'
import LogPanel from './LogPanel.vue'
import GameOverDialog from './GameOverDialog.vue'

const props = defineProps({
  game: { type: Object, required: true }
})

defineEmits(['backToLobby'])

const { timer, board, selectedSq, lastMoveSq, legalDestinations, moveHistory,
        humanColor, isThinking, phase, winner, isHumanTurn, onSquareClick,
        uciLog } = props.game
</script>

<style scoped>
.game-container {
  display: flex;
  gap: 16px;
  padding: 20px;
  max-width: 800px;
  margin: 0 auto;
  align-items: flex-start;
}
.main-col {
  display: flex;
  flex-direction: column;
  gap: 6px;
  flex-shrink: 0;
}
.side-col {
  flex: 1;
  min-width: 220px;
  max-width: 280px;
  max-height: calc(min(70vw, 480px) + 60px);
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.new-game-btn {
  padding: 10px 16px;
  background: #423d39;
  color: #ccc;
  border: 1px solid #555;
  border-radius: 4px;
  font-size: 0.9rem;
  font-weight: 600;
  cursor: pointer;
  transition: background 0.15s;
  margin-top: 4px;
}
.new-game-btn:hover {
  background: #55504b;
  color: #fff;
}

@media (max-width: 700px) {
  .game-container {
    flex-direction: column;
    align-items: center;
  }
  .side-col {
    width: min(70vw, 480px);
    max-height: 250px;
    min-width: 0;
    max-width: 100%;
  }
}
</style>

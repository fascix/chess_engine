<template>
  <div class="board">
    <template v-for="vr in 8" :key="vr">
      <Square
        v-for="vf in 8"
        :key="vf"
        :rank="boardRank(vr - 1)"
        :file="boardFile(vf - 1)"
        :piece="board[boardRank(vr - 1)]?.[boardFile(vf - 1)]"
        :is-selected="isSelected(boardRank(vr - 1), boardFile(vf - 1))"
        :is-last-move="isLastMove(boardRank(vr - 1), boardFile(vf - 1))"
        :is-legal-target="isLegalTarget(boardRank(vr - 1), boardFile(vf - 1))"
        :is-capture-target="isCaptureTarget(boardRank(vr - 1), boardFile(vf - 1))"
        :show-file="vr - 1 === 7"
        :show-rank="vf - 1 === 0"
        @click="$emit('squareClick', boardRank(vr - 1), boardFile(vf - 1))"
      />
    </template>
  </div>
</template>

<script setup>
import Square from './Square.vue'

const props = defineProps({
  board: { type: Array, required: true },
  selectedSq: { type: Array, default: null },
  lastMoveSq: { type: Array, default: null },
  legalDestinations: { type: Array, default: null },
  flipped: { type: Boolean, default: false }
})

defineEmits(['squareClick'])

function boardRank(visualRow) {
  return props.flipped ? 7 - visualRow : visualRow
}

function boardFile(visualCol) {
  return props.flipped ? 7 - visualCol : visualCol
}

function isSelected(rank, file) {
  return props.selectedSq && props.selectedSq[0] === rank && props.selectedSq[1] === file
}

function isLastMove(rank, file) {
  return props.lastMoveSq && props.lastMoveSq[0] === rank && props.lastMoveSq[1] === file
}

function isLegalTarget(rank, file) {
  return props.legalDestinations && props.legalDestinations[rank] && props.legalDestinations[rank][file]
}

function isCaptureTarget(rank, file) {
  return props.legalDestinations && props.legalDestinations[rank] && props.legalDestinations[rank][file] && props.board[rank]?.[file]
}
</script>

<style scoped>
.board {
  display: grid;
  grid-template-columns: repeat(8, 1fr);
  grid-template-rows: repeat(8, 1fr);
  width: min(70vw, 480px);
  height: min(70vw, 480px);
  border: 3px solid #423d39;
  border-radius: 4px;
  overflow: hidden;
  box-shadow: 0 4px 20px rgba(0,0,0,0.4);
}
</style>

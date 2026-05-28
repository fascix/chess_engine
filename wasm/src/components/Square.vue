<template>
  <div
    class="square"
    :class="[colorClass, { selected, 'last-move-light': isLastMove && colorClass === 'light', 'last-move-dark': isLastMove && colorClass === 'dark' }]"
    @click="$emit('click', rank, file)"
  >
    <span v-if="showFile" class="coord file">{{ fileLabel }}</span>
    <span v-if="showRank" class="coord rank">{{ rankLabel }}</span>
    <span v-if="isLegalTarget" class="move-dot" :class="{ capture: isCaptureTarget }"></span>
    <span v-if="piece" class="piece" :class="pieceColor">{{ PIECES[piece] }}</span>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { sqColor, PIECES } from '@/utils/board.js'

const props = defineProps({
  rank: Number,
  file: Number,
  piece: String,
  isSelected: Boolean,
  isLastMove: Boolean,
  isLegalTarget: Boolean,
  isCaptureTarget: Boolean,
  showFile: Boolean,
  showRank: Boolean
})

defineEmits(['click'])

const colorClass = computed(() => sqColor(props.rank, props.file))
const selected = computed(() => props.isSelected)
const pieceColor = computed(() => props.piece && (props.piece === props.piece.toUpperCase() ? 'white' : 'black'))
const fileLabel = computed(() => String.fromCharCode(97 + props.file))
const rankLabel = computed(() => 8 - props.rank)
</script>

<style scoped>
.square {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  user-select: none;
}
.square.light { background: #f0d9b5; }
.square.dark { background: #b58863; }
.square.selected { background: #829769; }
.square.last-move-light { background: #cdd26a; }
.square.last-move-dark { background: #aaa23a; }

.coord {
  position: absolute;
  font-size: 10px;
  font-weight: 600;
  pointer-events: none;
}
.coord.file { bottom: 1px; right: 3px; }
.coord.rank { top: 1px; left: 3px; }
.square.light .coord { color: #b58863; }
.square.dark .coord { color: #f0d9b5; }

.move-dot {
  position: absolute;
  width: 26%;
  height: 26%;
  border-radius: 50%;
  background: rgba(0,0,0,0.08);
  z-index: 1;
  pointer-events: none;
}
.move-dot.capture {
  width: 90%;
  height: 90%;
  background: transparent;
  border: 5px solid rgba(0,0,0,0.08);
  border-radius: 50%;
}

.piece {
  font-size: clamp(1.6rem, 7vw, 3.2rem);
  line-height: 1;
  z-index: 2;
  pointer-events: none;
  text-shadow: 0 1px 2px rgba(0,0,0,0.2);
}
.piece.white { color: #fff; }
.piece.black { color: #222; }
</style>

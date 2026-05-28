import { ref, onUnmounted } from 'vue'
import { formatTime } from '@/utils/board.js'

export function useTimer() {
  const whiteTimeMs = ref(600000)
  const blackTimeMs = ref(999999999)
  let turnStartMs = 0
  let intervalId = null
  let activeColor = null

  const whiteDisplay = ref('10:00')
  const blackDisplay = ref('∞')

  function currentMs(color) {
    const base = color === 'w' ? whiteTimeMs.value : blackTimeMs.value
    if (color === activeColor && turnStartMs > 0) {
      return Math.max(0, base - (Date.now() - turnStartMs))
    }
    return base
  }

  function updateDisplay() {
    whiteDisplay.value = formatTime(currentMs('w'))
    blackDisplay.value = formatTime(currentMs('b'))
  }

  function setTime(mode) {
    if (mode === 'blitz') {
      whiteTimeMs.value = 300000
      blackTimeMs.value = 300000
    } else if (mode === 'rapid') {
      whiteTimeMs.value = 600000
      blackTimeMs.value = 600000
    } else {
      whiteTimeMs.value = 999999999
      blackTimeMs.value = 999999999
    }
    updateDisplay()
  }

  function startTurn(color) {
    activeColor = color
    turnStartMs = Date.now()
    if (!intervalId) {
      intervalId = setInterval(updateDisplay, 200)
    }
  }

  function stopTurn() {
    if (turnStartMs === 0) return 0
    const elapsed = Date.now() - turnStartMs
    if (activeColor === 'w') {
      whiteTimeMs.value = Math.max(0, whiteTimeMs.value - elapsed)
    } else if (activeColor === 'b') {
      blackTimeMs.value = Math.max(0, blackTimeMs.value - elapsed)
    }
    turnStartMs = 0
    activeColor = null
    updateDisplay()
    return elapsed
  }

  function checkTimeout() {
    return currentMs('w') <= 0 || currentMs('b') <= 0
  }

  function timeoutWinner() {
    return currentMs('b') <= 0 ? 'white' : 'black'
  }

  function stop() {
    if (intervalId) {
      clearInterval(intervalId)
      intervalId = null
    }
    turnStartMs = 0
    activeColor = null
  }

  function reset() {
    stop()
    whiteTimeMs.value = 600000
    blackTimeMs.value = 999999999
    updateDisplay()
  }

  onUnmounted(stop)

  return {
    whiteTimeMs, blackTimeMs, whiteDisplay, blackDisplay,
    setTime, startTurn, stopTurn, checkTimeout, timeoutWinner,
    stop, reset, updateDisplay, currentMs
  }
}

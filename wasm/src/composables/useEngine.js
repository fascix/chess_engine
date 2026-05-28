import { ref, onUnmounted } from 'vue'

let instance = null

export function useEngine() {
  const ready = ref(false)
  const error = ref(null)
  let stdoutHandler = null
  let engine = null

  function init() {
    if (engine) return

    engine = new window.PallasChess()

    engine.onready = () => {
      ready.value = true
    }

    engine.onmessage = (text) => {
      if (stdoutHandler) stdoutHandler(text)
    }

    engine.onerror = (msg) => {
      error.value = msg
    }
  }

  function subscribeStdout(handler) {
    stdoutHandler = handler
  }

  function sendCommand(cmd) {
    if (!engine) return
    engine.sendCommand(cmd)
  }

  function getLegalMoves() {
    if (!engine) return ''
    return engine.getLegalMoves()
  }

  function requestLegalMoves(callback) {
    if (!engine) { callback(''); return }
    engine.requestLegalMoves(callback)
  }

  function sendPosition(fenOrStartpos, moves) {
    if (!engine) return
    engine.position(fenOrStartpos, moves)
  }

  function sendGo(params) {
    if (!engine) return
    engine.go(params)
  }

  function sendStop() {
    if (!engine) return
    engine.stop()
  }

  function destroy() {
    if (engine) {
      engine.destroy()
      engine = null
    }
    ready.value = false
    error.value = null
    stdoutHandler = null
    instance = null
  }

  onUnmounted(destroy)

  if (!instance) {
    instance = { init, ready, error, subscribeStdout, sendCommand, getLegalMoves, requestLegalMoves, sendPosition, sendGo, sendStop, destroy }
  }

  return instance
}

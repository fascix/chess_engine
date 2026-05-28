import { ref } from 'vue'
import { useEngine } from './useEngine.js'
import { useTimer } from './useTimer.js'
import { boardFromFen, cloneBoard, applyUciMove, isOwnPiece, STARTING_FEN } from '@/utils/board.js'

const MAX_LOG = 50

export function useGame() {
  const engine = useEngine()
  const timer = useTimer()

  const phase = ref('lobby')
  const board = ref([])
  const moveHistory = ref([])
  const humanColor = ref('w')
  const engineColor = ref('b')
  const isThinking = ref(false)
  const selectedSq = ref(null)
  const lastMoveSq = ref(null)
  const legalDestinations = ref(null)
  const moveCount = ref(0)
  const mode = ref('rapid')
  const winner = ref(null)
  const engineBusy = ref(false)
  const uciLog = ref([])

  function addLog(line) {
    uciLog.value = [...uciLog.value.slice(-(MAX_LOG - 1)), line]
  }

  function startGame(m, color) {
    mode.value = m
    humanColor.value = color
    engineColor.value = color === 'w' ? 'b' : 'w'
    phase.value = 'playing'
    moveHistory.value = []
    moveCount.value = 0
    selectedSq.value = null
    lastMoveSq.value = null
    legalDestinations.value = null
    isThinking.value = false
    winner.value = null
    engineBusy.value = false
    uciLog.value = []
    board.value = boardFromFen(STARTING_FEN)
    timer.reset()
    timer.setTime(m)
    engine.sendCommand('ucinewgame')
    engine.sendPosition('startpos', [])

    if (isHumanTurn()) {
      timer.startTurn(humanColor.value)
    } else {
      setTimeout(() => {
        timer.startTurn(engineColor.value)
        startEngineSearch()
      }, 100)
    }
  }

  function isHumanTurn() {
    return (humanColor.value === 'w' && moveCount.value % 2 === 0) ||
           (humanColor.value === 'b' && moveCount.value % 2 === 1)
  }

  function startEngineSearch() {
    if (phase.value !== 'playing') return
    if (isHumanTurn()) return
    isThinking.value = true

    if (mode.value === 'test') {
      engine.sendGo({ depth: 10 })
    } else {
      const wRemaining = Math.max(1, timer.currentMs('w'))
      const bRemaining = Math.max(1, timer.currentMs('b'))
      engine.sendGo({ wtime: wRemaining, btime: bRemaining, winc: 0, binc: 0 })
    }
  }

  function handleEngineOutput(text) {
    if (text.indexOf('info') === 0 && text.indexOf(' depth ') !== -1 && text.indexOf(' score ') !== -1) {
      addLog(text)
      return
    }

    if (text.indexOf('bestmove') === 0) {
      if (engineBusy.value) return
      engineBusy.value = true

      const parts = text.split(' ')
      const bestmove = parts[1]
      if (bestmove && bestmove !== '0000') {
        addLog('⏎ ' + text)
        timer.stopTurn()
        moveHistory.value = [...moveHistory.value, bestmove]
        moveCount.value++
        const nb = cloneBoard(board.value)
        applyUciMove(nb, bestmove)
        board.value = nb
        isThinking.value = false

        setTimeout(() => {
          engineBusy.value = false
          if (phase.value !== 'playing') return
          engine.sendPosition('startpos', moveHistory.value)
          if (!isHumanTurn()) {
            timer.startTurn(engineColor.value)
            startEngineSearch()
          } else {
            if (mode.value !== 'test') timer.startTurn(humanColor.value)
          }
        }, 0)
      } else {
        engineBusy.value = false
        isThinking.value = false
        endGame('draw')
      }
    }
  }

  function makeHumanMove(uci) {
    if (phase.value !== 'playing') return
    if (!isHumanTurn()) return
    timer.stopTurn()
    moveHistory.value = [...moveHistory.value, uci]
    moveCount.value++
    const nb = cloneBoard(board.value)
    applyUciMove(nb, uci)
    board.value = nb
    selectedSq.value = null
    legalDestinations.value = null

    if (timer.checkTimeout()) {
      endGame(timer.timeoutWinner() === humanColor.value ? 'human' : 'engine')
      return
    }

    setTimeout(() => {
      if (phase.value !== 'playing') return
      engine.sendPosition('startpos', moveHistory.value)
      timer.startTurn(engineColor.value)
      startEngineSearch()
    }, 30)
  }

  function onSquareClick(rank, file) {
    if (phase.value !== 'playing') return
    if (isThinking.value || !isHumanTurn()) return
    const piece = board.value[rank][file]

    if (selectedSq.value !== null) {
      const from = selectedSq.value
      if (from[0] === rank && from[1] === file) {
        selectedSq.value = null
        legalDestinations.value = null
        return
      }
      if (piece && isOwnPiece(piece, humanColor.value)) {
        selectedSq.value = [rank, file]
        queryLegalMoves(rank, file)
        return
      }
      if (legalDestinations.value && !legalDestinations.value[rank][file]) {
        selectedSq.value = null
        legalDestinations.value = null
        return
      }
      const uci = uciFromSquares(from[0], from[1], rank, file)
      selectedSq.value = null
      legalDestinations.value = null
      makeHumanMove(uci)
    } else {
      if (piece && isOwnPiece(piece, humanColor.value)) {
        selectedSq.value = [rank, file]
        queryLegalMoves(rank, file)
      }
    }
  }

  function queryLegalMoves(rank, file) {
    const movesStr = engine.getLegalMoves()
    if (!movesStr || movesStr.trim() === '') {
      legalDestinations.value = emptyGrid()
      if (phase.value === 'playing') endGame('engine')
      return
    }
    const fromUci = uciFromSquares(rank, file).substring(0, 2)
    const dests = emptyGrid()
    const allMoves = movesStr.split(' ')
    for (const m of allMoves) {
      if (m.substring(0, 2) === fromUci) {
        const tf = m.charCodeAt(2) - 97
        const tr = 8 - parseInt(m[3], 10)
        if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) dests[tr][tf] = true
      }
    }
    legalDestinations.value = dests
  }

  function emptyGrid() {
    const g = []
    for (let r = 0; r < 8; r++) {
      g[r] = []
      for (let f = 0; f < 8; f++) g[r][f] = false
    }
    return g
  }

  function uciFromSquares(fromRank, fromFile, toRank, toFile) {
    const fromFileCh = String.fromCharCode(97 + fromFile)
    const fromRankCh = 8 - fromRank
    const toFileCh = String.fromCharCode(97 + toFile)
    const toRankCh = 8 - toRank
    let uci = fromFileCh + fromRankCh + toFileCh + toRankCh
    const piece = board.value[fromRank][fromFile]
    if (piece && piece.toUpperCase() === 'P' && (toRank === 0 || toRank === 7)) {
      uci += 'q'
    }
    return uci
  }

  function endGame(result) {
    phase.value = 'gameover'
    isThinking.value = false
    timer.stop()
    if (result === 'draw') winner.value = 'draw'
    else if (result === 'human') winner.value = humanColor.value === 'w' ? 'white' : 'black'
    else if (result === 'engine') winner.value = engineColor.value === 'w' ? 'white' : 'black'
  }

  function backToLobby() {
    phase.value = 'lobby'
    winner.value = null
  }

  return {
    phase, board, moveHistory, humanColor, engineColor, isThinking,
    selectedSq, lastMoveSq, legalDestinations, moveCount, mode, winner,
    timer, engine, uciLog,
    startGame, isHumanTurn, onSquareClick, backToLobby, handleEngineOutput
  }
}

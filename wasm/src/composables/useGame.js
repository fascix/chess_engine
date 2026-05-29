import { ref } from 'vue'
import { useEngine } from './useEngine.js'
import { useTimer } from './useTimer.js'
import { boardFromFen, isOwnPiece, isInCheck, emptyGrid, STARTING_FEN } from '@/utils/board.js'
import { MoveApplier } from '@/services/MoveApplier.js'
import { DrawDetector } from '@/services/DrawDetector.js'

const MAX_LOG = 50

export function useGame() {
  const engine = useEngine()
  const timer = useTimer()
  const drawDetector = new DrawDetector()

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
  const capturedPieces = ref({ white: [], black: [] })
  const gameOverDismissed = ref(false)

  function addLog(line) {
    uciLog.value = [...uciLog.value.slice(-(MAX_LOG - 1)), line]
  }

  function resetState() {
    board.value = boardFromFen(STARTING_FEN)
    moveHistory.value = []
    moveCount.value = 0
    selectedSq.value = null
    lastMoveSq.value = null
    legalDestinations.value = null
    isThinking.value = false
    winner.value = null
    engineBusy.value = false
    uciLog.value = []
    capturedPieces.value = { white: [], black: [] }
    gameOverDismissed.value = false
    drawDetector.reset()
  }

  function startGame(m, color) {
    mode.value = m
    humanColor.value = color
    engineColor.value = color === 'w' ? 'b' : 'w'
    phase.value = 'playing'
    resetState()
    timer.reset()
    timer.setTime(m)
    engine.sendCommand('ucinewgame')
    engine.sendPosition('startpos', [])

    if (isHumanTurn()) timer.startTurn(humanColor.value)
    else setTimeout(() => { timer.startTurn(engineColor.value); startEngineSearch() }, 100)
  }

  function commitMove(uci) {
    const result = MoveApplier.apply(board.value, uci)
    board.value = result.board
    if (result.captured) {
      const k = result.captured === result.captured.toUpperCase() ? 'black' : 'white'
      capturedPieces.value = { ...capturedPieces.value, [k]: [...capturedPieces.value[k], result.captured] }
    }
    drawDetector.record(result.board, result.isPawn, result.captured)
    if (drawDetector.checkDraw(result.board)) { endGame('draw'); return true }
    return false
  }

  function isHumanTurn() {
    return (humanColor.value === 'w' && moveCount.value % 2 === 0) ||
           (humanColor.value === 'b' && moveCount.value % 2 === 1)
  }

  function startEngineSearch() {
    if (phase.value !== 'playing' || isHumanTurn()) return
    isThinking.value = true
    if (mode.value === 'test') engine.sendGo({ depth: 10 })
    else engine.sendGo({
      wtime: Math.max(1, timer.currentMs('w')),
      btime: Math.max(1, timer.currentMs('b')),
      winc: 0, binc: 0
    })
  }

  function afterEngineMove(bestmove) {
    addLog('⏎ bestmove ' + bestmove)
    timer.stopTurn()
    moveHistory.value = [...moveHistory.value, bestmove]
    moveCount.value++
    lastMoveSq.value = MoveApplier.lastMoveSquares(bestmove)
    if (commitMove(bestmove)) { engineBusy.value = false; return true }
    return false
  }

  function handleEngineOutput(text) {
    if (text.indexOf('info') === 0 && text.indexOf(' depth ') !== -1 && text.indexOf(' score ') !== -1) {
      addLog(text)
      return
    }

    if (text.indexOf('bestmove') !== 0 || engineBusy.value) return
    engineBusy.value = true
    const bestmove = text.split(' ')[1]

    if (bestmove && bestmove !== '0000') {
      isThinking.value = false
      if (afterEngineMove(bestmove)) return

      setTimeout(() => {
        engineBusy.value = false
        if (phase.value !== 'playing') return
        engine.sendPosition('startpos', moveHistory.value)
        if (!isHumanTurn()) { timer.startTurn(engineColor.value); startEngineSearch() }
        else {
          if (mode.value !== 'test') timer.startTurn(humanColor.value)
          engine.requestLegalMoves(function (movesStr) {
            if (!movesStr || movesStr.trim() === '') {
              if (isInCheck(board.value, humanColor.value)) endGame('engine')
              else endGame('draw')
            }
          })
        }
      }, 0)
    } else {
      engineBusy.value = false
      isThinking.value = false
      if (isInCheck(board.value, engineColor.value)) endGame('human')
      else endGame('draw')
    }
  }

  function makeHumanMove(uci) {
    if (phase.value !== 'playing' || !isHumanTurn()) return
    timer.stopTurn()
    moveHistory.value = [...moveHistory.value, uci]
    moveCount.value++
    lastMoveSq.value = MoveApplier.lastMoveSquares(uci)
    selectedSq.value = null
    legalDestinations.value = null
    if (commitMove(uci)) return

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
    if (phase.value !== 'playing' || isThinking.value || !isHumanTurn()) return
    const piece = board.value[rank][file]

    if (selectedSq.value !== null) {
      const from = selectedSq.value
      if (from[0] === rank && from[1] === file) { selectedSq.value = null; legalDestinations.value = null; return }
      if (piece && isOwnPiece(piece, humanColor.value)) { selectedSq.value = [rank, file]; queryLegalMoves(rank, file); return }
      if (legalDestinations.value && !legalDestinations.value[rank][file]) { selectedSq.value = null; legalDestinations.value = null; return }
      selectedSq.value = null
      legalDestinations.value = null
      makeHumanMove(MoveApplier.uciFromSquares(board.value, from[0], from[1], rank, file))
    } else if (piece && isOwnPiece(piece, humanColor.value)) {
      selectedSq.value = [rank, file]
      queryLegalMoves(rank, file)
    }
  }

  function queryLegalMoves(rank, file) {
    const movesStr = engine.getLegalMoves()
    if (!movesStr || movesStr.trim() === '') {
      legalDestinations.value = emptyGrid()
      if (phase.value === 'playing') {
        if (isInCheck(board.value, humanColor.value)) endGame('engine')
        else endGame('draw')
      }
      return
    }
    const fromPrefix = MoveApplier.uciFromSquares(board.value, rank, file, rank, file).substring(0, 2)
    const dests = emptyGrid()
    for (const m of movesStr.split(' ')) {
      if (m.substring(0, 2) === fromPrefix) {
        const tf = m.charCodeAt(2) - 97
        const tr = 8 - parseInt(m[3], 10)
        if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) dests[tr][tf] = true
      }
    }
    legalDestinations.value = dests
  }

  function endGame(result) {
    phase.value = 'gameover'
    gameOverDismissed.value = false
    isThinking.value = false
    timer.stop()
    if (result === 'draw') winner.value = 'draw'
    else if (result === 'human') winner.value = humanColor.value === 'w' ? 'white' : 'black'
    else if (result === 'engine') winner.value = engineColor.value === 'w' ? 'white' : 'black'
  }

  function dismissGameOver() { gameOverDismissed.value = true }

  function backToLobby() {
    phase.value = 'lobby'
    winner.value = null
    gameOverDismissed.value = false
  }

  return {
    phase, board, moveHistory, humanColor, engineColor, isThinking,
    selectedSq, lastMoveSq, legalDestinations, moveCount, mode, winner,
    timer, engine, uciLog, capturedPieces, gameOverDismissed,
    startGame, isHumanTurn, onSquareClick, backToLobby, handleEngineOutput, dismissGameOver
  }
}

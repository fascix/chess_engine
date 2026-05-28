export const PIECES = {
  K: '♔', Q: '♕', R: '♖', B: '♗', N: '♘', P: '♙',
  k: '♚', q: '♛', r: '♜', b: '♝', n: '♞', p: '♟'
}

export const STARTING_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR'

export function boardFromFen(fen) {
  const board = []
  const rows = fen.split('/')
  for (let r = 0; r < 8; r++) {
    const row = []
    for (let i = 0; i < rows[r].length; i++) {
      const ch = rows[r][i]
      if (ch >= '1' && ch <= '8') {
        const empty = parseInt(ch, 10)
        for (let e = 0; e < empty; e++) row.push('')
      } else {
        row.push(ch)
      }
    }
    board.push(row)
  }
  return board
}

export function sqToUci(rank, file) {
  return String.fromCharCode(97 + file) + (8 - rank)
}

export function uciToSq(uci) {
  const file = uci.charCodeAt(0) - 97
  const rank = 8 - parseInt(uci[1], 10)
  return [rank, file]
}

export function cloneBoard(board) {
  return board.map(row => [...row])
}

export function applyUciMove(board, uci) {
  const fromFile = uci.charCodeAt(0) - 97
  const fromRank = 8 - parseInt(uci[1], 10)
  const toFile = uci.charCodeAt(2) - 97
  const toRank = 8 - parseInt(uci[3], 10)
  const promotion = uci.length > 4 ? uci[4] : null

  const piece = board[fromRank][fromFile]
  if (!piece) return

  if (piece.toUpperCase() === 'K' && Math.abs(toFile - fromFile) === 2) {
    const rookFrom = toFile > fromFile ? 7 : 0
    const rookTo = toFile > fromFile ? 5 : 3
    board[fromRank][rookTo] = board[fromRank][rookFrom]
    board[fromRank][rookFrom] = ''
  }

  if (piece.toUpperCase() === 'P' && fromFile !== toFile && board[toRank][toFile] === '') {
    board[fromRank][toFile] = ''
  }

  board[toRank][toFile] = promotion || piece
  board[fromRank][fromFile] = ''
}

export function sqColor(rank, file) {
  return (rank + file) % 2 === 0 ? 'light' : 'dark'
}

export function isOwnPiece(piece, humanColor) {
  if (!piece) return false
  if (humanColor === 'w') return piece === piece.toUpperCase()
  return piece === piece.toLowerCase()
}

export function formatTime(ms) {
  if (ms >= 999999999) return '∞'
  const totalSec = Math.max(0, Math.floor(ms / 1000))
  const min = Math.floor(totalSec / 60)
  const sec = totalSec % 60
  return `${min}:${String(sec).padStart(2, '0')}`
}

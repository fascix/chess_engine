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

export function emptyGrid() {
  const g = []
  for (let r = 0; r < 8; r++) {
    g[r] = []
    for (let f = 0; f < 8; f++) g[r][f] = false
  }
  return g
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

export function isInCheck(board, color) {
  let kr = -1, kf = -1
  const king = color === 'w' ? 'K' : 'k'
  for (let r = 0; r < 8; r++) {
    for (let f = 0; f < 8; f++) {
      if (board[r][f] === king) { kr = r; kf = f; break }
    }
    if (kr >= 0) break
  }
  if (kr < 0) return true

  const opp = color === 'w' ? 'b' : 'w'
  for (let r = 0; r < 8; r++) {
    for (let f = 0; f < 8; f++) {
      const p = board[r][f]
      if (!p) continue
      if ((p === p.toUpperCase() ? 'w' : 'b') !== opp) continue
      if (attacks(board, r, f, kr, kf, p)) return true
    }
  }
  return false
}

function attacks(board, fr, ff, tr, tf, p) {
  const dr = tr - fr, df = tf - ff
  const adr = Math.abs(dr), adf = Math.abs(df)
  const t = p.toUpperCase()
  if (t === 'P') return dr === (p === 'P' ? -1 : 1) && adf === 1
  if (t === 'N') return (adr === 2 && adf === 1) || (adr === 1 && adf === 2)
  if (t === 'K') return adr <= 1 && adf <= 1 && (adr + adf) > 0
  if (t === 'B' || t === 'Q') {
    if (adr === adf && adr > 0) {
      const sdr = dr > 0 ? 1 : -1, sdf = df > 0 ? 1 : -1
      let r = fr + sdr, f = ff + sdf
      while (r !== tr && f !== tf) { if (board[r][f]) return false; r += sdr; f += sdf }
      return true
    }
  }
  if (t === 'R' || t === 'Q') {
    if ((dr === 0 && df !== 0) || (dr !== 0 && df === 0)) {
      if (dr === 0) {
        const sdf = df > 0 ? 1 : -1
        let f = ff + sdf
        while (f !== tf) { if (board[fr][f]) return false; f += sdf }
      } else {
        const sdr = dr > 0 ? 1 : -1
        let r = fr + sdr
        while (r !== tr) { if (board[r][ff]) return false; r += sdr }
      }
      return true
    }
  }
  return false
}

export function positionKey(board) {
  return board.map(row => row.map(c => c || '.').join('')).join('/')
}

export function insufficientMaterial(board) {
  let w = 0, b = 0
  let wb = 0, bb = 0, wn = 0, bn = 0
  for (let r = 0; r < 8; r++) {
    for (let f = 0; f < 8; f++) {
      const p = board[r][f]
      if (!p) continue
      const t = p.toUpperCase()
      if (t === 'K') continue
      if (p === p.toUpperCase()) { w++; if (t === 'B') wb++; else if (t === 'N') wn++ }
      else { b++; if (t === 'B') bb++; else if (t === 'N') bn++ }
    }
  }
  if (w === 0 && b === 0) return true
  if (w === 1 && wb === 1 && b === 0) return true
  if (w === 0 && b === 1 && bb === 1) return true
  if (w === 1 && wn === 1 && b === 0) return true
  if (w === 0 && b === 1 && bn === 1) return true
  return false
}

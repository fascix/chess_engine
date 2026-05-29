export class MoveApplier {
  static apply(board, uci) {
    const fromFile = uci.charCodeAt(0) - 97
    const fromRank = 8 - parseInt(uci[1], 10)
    const toFile = uci.charCodeAt(2) - 97
    const toRank = 8 - parseInt(uci[3], 10)
    const nb = board.map(row => [...row])
    const piece = nb[fromRank][fromFile]

    let captured = nb[toRank][toFile]
    if (!captured && piece && piece.toUpperCase() === 'P' && fromFile !== toFile) {
      captured = nb[fromRank][toFile]
    }

    if (piece && piece.toUpperCase() === 'K' && Math.abs(toFile - fromFile) === 2) {
      const rookFrom = toFile > fromFile ? 7 : 0
      const rookTo = toFile > fromFile ? 5 : 3
      nb[fromRank][rookTo] = nb[fromRank][rookFrom]
      nb[fromRank][rookFrom] = ''
    }

    if (piece && piece.toUpperCase() === 'P' && fromFile !== toFile && nb[toRank][toFile] === '') {
      nb[fromRank][toFile] = ''
    }

    const promotion = uci.length > 4 ? uci[4] : null
    nb[toRank][toFile] = promotion || piece
    nb[fromRank][fromFile] = ''

    return { board: nb, isPawn: !!(piece && piece.toUpperCase() === 'P'), captured: captured || null }
  }

  static lastMoveSquares(uci) {
    return [
      [8 - parseInt(uci[1], 10), uci.charCodeAt(0) - 97],
      [8 - parseInt(uci[3], 10), uci.charCodeAt(2) - 97]
    ]
  }

  static uciFromSquares(board, fromRank, fromFile, toRank, toFile) {
    const uci = String.fromCharCode(97 + fromFile) + (8 - fromRank) +
                String.fromCharCode(97 + toFile) + (8 - toRank)
    const piece = board[fromRank][fromFile]
    if (piece && piece.toUpperCase() === 'P' && (toRank === 0 || toRank === 7)) return uci + 'q'
    return uci
  }
}

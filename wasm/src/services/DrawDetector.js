import { positionKey, insufficientMaterial } from '@/utils/board.js'

export class DrawDetector {
  constructor() {
    this.reset()
  }

  record(board, isPawn, captured) {
    if (isPawn || captured) this.halfMoveClock = 0
    else this.halfMoveClock++
    this.history.push(positionKey(board))
  }

  checkDraw(board) {
    const key = positionKey(board)
    return this.history.filter(k => k === key).length >= 3 ||
           this.halfMoveClock >= 100 ||
           insufficientMaterial(board)
  }

  reset() {
    this.history = []
    this.halfMoveClock = 0
  }
}

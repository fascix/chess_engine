var PallasChess = (function () {
  function PallasChess() {
    var self = this;
    self._ready = false;
    self._worker = null;
    self.onmessage = null;
    self.onready = null;
    self.onerror = null;
    self._cmdQueue = [];
    self._processingCmd = false;
    self._legalMovesCache = '';
    self._legalCallbacks = [];

    var selfRef = self;

    Promise.all([
      fetch('pallas.js').then(function (r) { return r.text(); }),
      fetch('pallas-worker.js').then(function (r) { return r.text(); })
    ]).then(function (results) {
      var pallasCode = results[0];
      var workerCode = results[1];
      var combined = pallasCode + '\n' + workerCode;
      var blob = new Blob([combined], { type: 'application/javascript' });
      selfRef._worker = new Worker(URL.createObjectURL(blob));

      selfRef._worker.onmessage = function (e) {
        var msg = e.data;
        if (msg.type === 'ready') {
          selfRef._ready = true;
          if (selfRef.onready) selfRef.onready();
        } else if (msg.type === 'stdout') {
          if (selfRef.onmessage) selfRef.onmessage(msg.text);
        } else if (msg.type === 'done') {
          selfRef._processingCmd = false;
          selfRef._flushQueue();
        } else if (msg.type === 'legal_moves') {
          selfRef._legalMovesCache = msg.moves;
          for (var i = 0; i < selfRef._legalCallbacks.length; i++)
            selfRef._legalCallbacks[i](msg.moves);
          selfRef._legalCallbacks = [];
        } else if (msg.type === 'error') {
          console.error('[Worker]', msg.message);
          if (selfRef.onerror) selfRef.onerror(msg.message);
        }
      };

      selfRef._worker.onerror = function (e) {
        console.error('Worker error:', e.message, e.filename, e.lineno);
        if (selfRef.onerror) selfRef.onerror('Worker error: ' + (e.message || 'unknown'));
      };
    }).catch(function (err) {
      console.error('Failed to load engine scripts:', err);
      if (selfRef.onerror) selfRef.onerror('Failed to load engine scripts: ' + err.message);
    });
  }

  PallasChess.prototype.sendCommand = function (cmd) {
    if (!this._worker) { this._cmdQueue.push(cmd); return; }
    this._cmdQueue.push(cmd);
    if (cmd.indexOf('position ') === 0 || cmd === 'ucinewgame' || cmd === 'isready') {
      this._legalMovesCache = '';
    }
    this._flushQueue();
  };

  PallasChess.prototype._flushQueue = function () {
    if (this._processingCmd || this._cmdQueue.length === 0) return;
    this._processingCmd = true;
    var cmd = this._cmdQueue.shift();
    this._worker.postMessage({ type: 'cmd', cmd: cmd });
  };

  PallasChess.prototype.getLegalMoves = function () {
    return this._legalMovesCache;
  };

  PallasChess.prototype.requestLegalMoves = function (callback) {
    if (this._legalMovesCache) {
      if (callback) callback(this._legalMovesCache);
      return;
    }
    if (callback) this._legalCallbacks.push(callback);
    this._worker.postMessage({ type: 'get_legal_moves' });
  };

  PallasChess.prototype.stop = function () {
    if (this._worker) {
      this._worker.postMessage({ type: 'stop' });
    }
  };

  PallasChess.prototype.uci = function () { this.sendCommand('uci'); };
  PallasChess.prototype.isready = function () { this.sendCommand('isready'); };
  PallasChess.prototype.ucinewgame = function () { this.sendCommand('ucinewgame'); };
  PallasChess.prototype.quit = function () { this.sendCommand('quit'); };

  PallasChess.prototype.position = function (fenOrStartpos, moves) {
    var cmd = 'position ' + fenOrStartpos;
    if (moves && moves.length > 0) cmd += ' moves ' + moves.join(' ');
    this.sendCommand(cmd);
  };

  PallasChess.prototype.go = function (params) {
    var cmd = 'go';
    if (params) {
      if (params.depth) cmd += ' depth ' + params.depth;
      if (params.movetime) cmd += ' movetime ' + params.movetime;
      if (params.wtime !== undefined) cmd += ' wtime ' + params.wtime;
      if (params.btime !== undefined) cmd += ' btime ' + params.btime;
      if (params.winc) cmd += ' winc ' + params.winc;
      if (params.binc) cmd += ' binc ' + params.binc;
      if (params.infinite) cmd += ' infinite';
    }
    this.sendCommand(cmd);
  };

  PallasChess.prototype.destroy = function () {
    if (this._worker) {
      this._worker.terminate();
      this._worker = null;
    }
    this._ready = false;
  };

  return PallasChess;
})();

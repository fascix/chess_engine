var engine = null;
var localModule = {};

localModule.print = function (text) {
  self.postMessage({ type: 'stdout', text: text });
};

localModule.printErr = function (text) {
  self.postMessage({ type: 'stdout', text: text });
};

PallasEngine(localModule).then(function (mod) {
  engine = mod;
  mod._pallas_init();
  self.postMessage({ type: 'ready' });
}).catch(function (err) {
  self.postMessage({ type: 'error', message: 'Init failed: ' + (err.message || String(err)) });
});

self.onmessage = function (e) {
  var msg = e.data;

  if (msg.type === 'cmd') {
    if (!engine) return;
    try {
      engine.ccall('pallas_uci_command', 'void', ['string'], [msg.cmd]);
      if (msg.cmd.indexOf('position ') === 0 || msg.cmd === 'ucinewgame' || msg.cmd === 'isready') {
        var moves = engine.ccall('pallas_get_legal_moves', 'string', [], []);
        self.postMessage({ type: 'legal_moves', moves: moves });
      }
    } catch (err) {
      self.postMessage({ type: 'error', message: 'Cmd error: ' + (err.message || String(err)) });
    }
    self.postMessage({ type: 'done' });

  } else if (msg.type === 'get_legal_moves') {
    if (!engine) return;
    try {
      var moves = engine.ccall('pallas_get_legal_moves', 'string', [], []);
      self.postMessage({ type: 'legal_moves', moves: moves });
    } catch (err) {
      self.postMessage({ type: 'legal_moves', moves: '' });
    }

  } else if (msg.type === 'stop') {
    if (engine) {
      engine.ccall('pallas_uci_command', 'void', ['string'], ['stop']);
    }
  }
};

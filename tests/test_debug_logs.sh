#!/bin/bash

echo "=== Test des logs DEBUG ==="
echo ""

(
  echo "uci"
  sleep 0.1
  echo "isready"
  sleep 0.1
  echo "position startpos"
  sleep 0.1
  echo "go wtime 5000 btime 5000 winc 50 binc 50"
  sleep 2
  echo "quit"
) | ./chess_engine_debug 2>&1 | head -100


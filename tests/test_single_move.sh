#!/bin/bash

echo "=== Test d'un seul coup avec chronométrage ==="
echo ""

# Mesurer le temps d'un seul coup
(
  echo "uci"
  echo "isready"
  echo "position startpos"
  echo "go wtime 10100 btime 10100 winc 100 binc 100"
  sleep 1
  echo "quit"
) | /usr/bin/time -p ./chess_engine 2>&1


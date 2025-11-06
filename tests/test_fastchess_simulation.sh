#!/bin/bash

echo "=== Simulation d'une partie fastchess ==="
echo ""
echo "Temps de réflexion attendu par coup : environ 250-300ms"
echo ""

(
  echo "uci"
  sleep 0.1
  echo "isready"
  sleep 0.1
  echo "ucinewgame"
  sleep 0.1
  echo "isready"
  sleep 0.1
  
  # Premier coup des blancs
  echo "position startpos"
  echo "go wtime 10100 btime 10100 winc 100 binc 100"
  sleep 1
  
  # Deuxième coup (après e2e4 par exemple)
  echo "position startpos moves e2e4"
  echo "go wtime 9850 btime 10100 winc 100 binc 100"
  sleep 1
  
  # Troisième coup
  echo "position startpos moves e2e4 e7e5"
  echo "go wtime 9700 btime 9850 winc 100 binc 100"
  sleep 1
  
  echo "quit"
) | time ./chess_engine 2>&1


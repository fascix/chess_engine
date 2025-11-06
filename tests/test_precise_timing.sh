#!/bin/bash

echo "=== Test de timing précis pour un seul coup ==="
echo ""
echo "Paramètres: wtime=10100ms, btime=10100ms, winc=100ms, binc=100ms"
echo "Temps attendu: ~100ms (10100 / (50*2) + 75 = 101 + 75 = 176ms environ)"
echo ""

# Test sans sleep inutiles
{
  echo "uci"
  echo "isready"
  echo "ucinewgame"
  echo "isready"
  echo "position startpos"
  echo "go wtime 10100 btime 10100 winc 100 binc 100"
  # Attendre que bestmove soit retourné (max 2 sec)
  sleep 2
  echo "quit"
} | ./chess_engine 2>&1 | grep -E "(bestmove|info depth)"

echo ""
echo "=== Test avec moins de temps (wtime=1000ms) ==="
{
  echo "uci"
  echo "isready"
  echo "position startpos"
  echo "go wtime 1000 btime 1000 winc 0 binc 0"
  sleep 1
  echo "quit"
} | ./chess_engine 2>&1 | grep -E "(bestmove|info depth)"


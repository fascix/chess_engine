#!/bin/bash

# Test pour voir l'évolution après quelques coups

echo "=== TEST APRÈS QUELQUES COUPS ==="
echo ""

{
  echo "uci"
  sleep 0.5
  echo "isready"
  sleep 0.5
  echo "ucinewgame"
  sleep 0.5
  # Après e2e4 e7e5
  echo "position startpos moves e2e4 e7e5"
  sleep 0.5
  echo "go depth 4"
  sleep 5
  echo "quit"
} | ./chess_engine_debug 2>&1 | grep -A 30 "=== RECHERCHE"


#!/bin/bash

echo "=== Test de gestion du temps UCI ==="
echo ""

# Test 1: avec wtime/btime
echo "Test 1: Position de départ avec wtime 10000 btime 10000 winc 100 binc 100"
echo "(devrait prendre environ 250-300ms)"
(
  echo "uci"
  sleep 0.1
  echo "isready"
  sleep 0.1
  echo "position startpos"
  sleep 0.1
  echo "go wtime 10000 btime 10000 winc 100 binc 100"
  sleep 3
  echo "quit"
) | timeout 5 ./chess_engine

echo ""
echo "Test 2: movetime fixe de 500ms"
echo "(devrait prendre environ 500ms)"
(
  echo "uci"
  sleep 0.1
  echo "isready"
  sleep 0.1
  echo "position startpos"
  sleep 0.1
  echo "go movetime 500"
  sleep 2
  echo "quit"
) | timeout 5 ./chess_engine

echo ""
echo "Test 3: depth fixe de 5"
echo "(temps variable selon la profondeur)"
(
  echo "uci"
  sleep 0.1
  echo "isready"
  sleep 0.1
  echo "position startpos"
  sleep 0.1
  echo "go depth 5"
  sleep 5
  echo "quit"
) | timeout 10 ./chess_engine


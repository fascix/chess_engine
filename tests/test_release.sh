#!/bin/bash

echo "Test de l'engine en version release (sans TT)"
echo ""

{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "ucinewgame"
  sleep 0.3
  echo "position startpos"
  sleep 0.3
  echo "go depth 5"
  sleep 10
  echo "quit"
} | ./chess_engine

echo ""
echo "Done!"


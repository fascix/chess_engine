#!/bin/bash

# Script de test pour diagnostiquer le comportement de l'engine

echo "=== TEST DEBUG CHESS ENGINE ==="
echo ""
echo "Lancement de l'engine en mode debug..."
echo "Les logs de débogage seront affichés sur stderr"
echo ""
echo "Position de départ - analysons les 5 premiers coups"
echo ""

# Commandes UCI pour tester
{
  echo "uci"
  sleep 0.5
  echo "isready"
  sleep 0.5
  echo "ucinewgame"
  sleep 0.5
  echo "position startpos"
  sleep 0.5
  echo "go depth 4"
  sleep 5
  echo "quit"
} | ./chess_engine_debug 2>&1 | tee debug_output.log

echo ""
echo "=== ANALYSE TERMINÉE ==="
echo "Les résultats sont dans debug_output.log"
echo ""
echo "Recherche des meilleurs coups détectés:"
grep "MEILLEUR COUP" debug_output.log
echo ""
echo "Recherche des évaluations:"
grep "EVAL" debug_output.log | head -20


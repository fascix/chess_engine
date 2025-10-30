#!/bin/bash

echo "=== TEST: Vérification du Time Management Amélioré ==="
echo ""
echo "Test 1: Position initiale avec 10s de temps"
echo "Attendu: Le moteur devrait chercher à depth 6+ (au lieu de 3-4)"
echo ""

# Test avec 10 secondes
echo "uci" | ./chess_engine_fixed
echo "isready" | ./chess_engine_fixed
echo "position startpos" | ./chess_engine_fixed
echo "go wtime 10000 btime 10000 winc 100 binc 100" | timeout 12 ./chess_engine_fixed | grep "^info depth"

echo ""
echo "Test 2: Vérifier qu'un cavalier sur a4 est mal évalué"
echo "Position: Après 1. Nc3 Nc6 2. Na4"
echo ""

echo "position startpos moves b1c3 b8c6 c3a4" | ./chess_engine_fixed
echo "go movetime 1000" | timeout 2 ./chess_engine_fixed | grep "score"

echo ""
echo "=== FIN DES TESTS ==="


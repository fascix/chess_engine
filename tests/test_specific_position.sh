#!/bin/bash

echo "=== TEST: Position spécifique où le moteur est faible ==="
echo ""

# Position après 8 coups de la partie Round 47
# Le moteur joue Rb1 {+4.22/3 0.303s} - seulement depth 3!
POSITION="rnbqk2r/pbpppppp/1p3n2/8/2P5/5NP1/PP1PPP1P/RNBQKB1R b KQkq - 0 8"

echo "Position: Après 8. g3 (notre moteur a les blancs)"
echo "FEN: $POSITION"
echo ""
echo "Test 1: Recherche avec 2 secondes"
echo "position fen $POSITION" | ./chess_engine
echo "go movetime 2000" | ./chess_engine | grep "^info"

echo ""
echo "Test 2: Recherche avec 5 secondes"
echo "position fen $POSITION" | ./chess_engine  
echo "go movetime 5000" | ./chess_engine | grep "^info"

echo ""
echo "Test 3: Debug - compter les noeuds"
echo "position fen $POSITION" | ./chess_engine_debug 2>&1 | tail -20
echo "go movetime 1000" | ./chess_engine_debug 2>&1 | grep -E "(depth|nodes|NMP|LMR)" | head -50


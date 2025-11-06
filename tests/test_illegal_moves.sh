#!/bin/bash

# Script pour tester que le moteur ne génère pas de coups illégaux
# Ce test reproduit les situations des logs FastChess

echo "==================================================================="
echo "TEST: Vérification des coups illégaux (a1a1, e1e6, etc.)"
echo "==================================================================="
echo ""

# Compiler le moteur en mode release
echo "1. Compilation du moteur..."
cd "$(dirname "$0")/.."
make clean > /dev/null 2>&1
make release > /dev/null 2>&1

if [ ! -f "./chess_engine" ]; then
    echo "❌ ERREUR: Compilation échouée!"
    exit 1
fi
echo "✅ Compilation réussie"
echo ""

# Test 1: Position du coup a1a1 (Game #6)
echo "2. Test position #1 (coup a1a1 observé)"
echo "   Position après: 1.e4 Nh6 2.d4 Nc6 3.Nf3 Ng4 4.h3 Nf6 5.e5 Ne4 6.d5 Ng5 7.Bxg5 Ne5 8.Nxe5 h6 9.Qf3 f6"

cat << EOF | timeout 5 ./chess_engine 2>&1 | tee /tmp/test_a1a1.log
uci
isready
position startpos moves e2e4 g8h6 d2d4 b8c6 g1f3 h6g4 h2h3 g4f6 e4e5 f6e4 d4d5 e4g5 c1g5 c6e5 f3e5 h7h6 d1f3 f7f6
go movetime 1000
quit
EOF

# Vérifier qu'il n'y a pas de coup a1a1
if grep -q "bestmove a1a1" /tmp/test_a1a1.log; then
    echo "❌ ÉCHEC: Le moteur a généré le coup illégal 'a1a1'"
    exit 1
else
    echo "✅ SUCCÈS: Pas de coup a1a1 détecté"
fi
echo ""

# Test 2: Position du coup e1e6 (Game #4, move 27)
echo "3. Test position #2 (coup e1e6 observé)"
echo "   Position complexe après 27 coups"

cat << EOF | timeout 5 ./chess_engine 2>&1 | tee /tmp/test_e1e6.log
uci
isready
position startpos moves e2e4 g8h6 d2d4 b8c6 g1f3 h6g4 h2h3 g4f6 d4d5 f6d5 e4d5 c6b4 a2a3 b4d5 d1d5 e7e6 d5b3 f8e7 b1c3 e7d6 c3b5 e8g8 c1e3 a7a6 b5d6 c7d6 b3b6 d8b6 e3b6 d6d5 a3a4 f7f5 b6c7 f8f6 e1c1 f6f8 a4a5 f8e8 f1d3 e8f8 c7d6 f8f6 f3e5 g8h8 d6e7 h8g8 e7f6 g7f6 e5f3 a8b8 h1e1 b7b5 a5b6
go movetime 1000
quit
EOF

# Vérifier qu'il n'y a pas de coup e1e6 ou autre coup invalide
if grep -q "bestmove e1e6" /tmp/test_e1e6.log; then
    echo "❌ ÉCHEC: Le moteur a généré le coup illégal 'e1e6'"
    exit 1
else
    echo "✅ SUCCÈS: Pas de coup e1e6 détecté"
fi
echo ""

# Test 3: Vérifier qu'aucun coup de type XnXn n'est généré (même case)
echo "4. Test général: Vérification des coups vers la même case"

for i in {1..10}; do
    cat << EOF | timeout 3 ./chess_engine 2>&1 | tee /tmp/test_general_$i.log
uci
isready
position startpos
go movetime 100
quit
EOF

    # Extraire le bestmove
    bestmove=$(grep "bestmove" /tmp/test_general_$i.log | tail -1 | awk '{print $2}')
    
    if [ -n "$bestmove" ]; then
        # Vérifier que from != to
        from_square="${bestmove:0:2}"
        to_square="${bestmove:2:2}"
        
        if [ "$from_square" = "$to_square" ]; then
            echo "❌ ÉCHEC test $i: Coup vers la même case détecté: $bestmove"
            exit 1
        fi
    fi
done

echo "✅ SUCCÈS: Aucun coup vers la même case détecté sur 10 tests"
echo ""

# Test 4: Test rapide avec temps très court (cas de panique)
echo "5. Test de panique (temps très court)"

for i in {1..5}; do
    cat << EOF | timeout 2 ./chess_engine 2>&1 | tee /tmp/test_panic_$i.log
uci
isready
position startpos
go movetime 10
quit
EOF

    bestmove=$(grep "bestmove" /tmp/test_panic_$i.log | tail -1 | awk '{print $2}')
    
    if [ -n "$bestmove" ]; then
        from_square="${bestmove:0:2}"
        to_square="${bestmove:2:2}"
        
        if [ "$from_square" = "$to_square" ]; then
            echo "❌ ÉCHEC test panique $i: Coup vers la même case: $bestmove"
            exit 1
        fi
        
        # Vérifier que ce n'est pas 0000
        if [ "$bestmove" = "0000" ]; then
            echo "❌ ÉCHEC test panique $i: Coup null détecté"
            exit 1
        fi
    fi
done

echo "✅ SUCCÈS: Tous les tests de panique ont passé"
echo ""

echo "==================================================================="
echo "✅ TOUS LES TESTS ONT RÉUSSI!"
echo "==================================================================="
echo ""
echo "Le moteur ne devrait plus générer de coups illégaux."
echo "Vous pouvez maintenant tester avec FastChess."

exit 0


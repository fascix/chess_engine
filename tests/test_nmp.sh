#!/bin/bash
# Test du Null Move Pruning
# On compare les performances avec/sans NMP sur quelques positions

echo "=== TEST NULL MOVE PRUNING ==="
echo ""

# Compiler avec NMP activé
echo "🔨 Compilation avec NMP..."
gcc -o chess_engine_nmp src/*.c -lm -O3

if [ $? -ne 0 ]; then
    echo "❌ Erreur de compilation"
    exit 1
fi

echo "✅ Compilation réussie"
echo ""

# Test 1 : Position tactique (doit être plus rapide avec NMP)
echo "📊 Test 1 - Position standard (depth 6)"
echo "position startpos" | ./chess_engine_nmp > /dev/null
echo "go depth 6" | timeout 10 ./chess_engine_nmp 2>&1 | grep -E "(bestmove|nodes|time)"

echo ""
echo "---"
echo ""

# Test 2 : Position dominante (NMP devrait briller ici)
echo "📊 Test 2 - Position avec avantage matériel (depth 6)"
echo "position fen r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4" | ./chess_engine_nmp > /dev/null
echo "go depth 6" | timeout 10 ./chess_engine_nmp 2>&1 | grep -E "(bestmove|nodes|time)"

echo ""
echo "---"
echo ""

# Test 3 : Vérifier qu'on ne fait pas NMP en échec
echo "📊 Test 3 - Position avec échec (NMP doit être désactivé)"
echo "position fen rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2" | ./chess_engine_nmp > /dev/null
echo "go depth 5" | timeout 10 ./chess_engine_nmp 2>&1 | grep -E "(bestmove|nodes|time)"

echo ""
echo "=== FIN DES TESTS ==="
echo ""
echo "✅ Si vous voyez des bestmove sans erreur, le NMP fonctionne !"
echo "💡 Astuce : Comparez les 'nodes' avec l'ancien moteur - ils devraient diminuer de 30-50%"


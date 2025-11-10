#!/bin/bash

# Script de test rapide pour vérifier que V3 bat bien V2 avec le fix appliqué

echo "=========================================="
echo "Test de Validation : V2 vs V3"
echo "=========================================="
echo ""

# Vérifier que les binaires existent
if [ ! -f "versions/v2_build/chess_engine_v2" ]; then
    echo "❌ V2 n'existe pas. Compilez avec: make v2"
    exit 1
fi

if [ ! -f "versions/v3_build/chess_engine_v3" ]; then
    echo "❌ V3 n'existe pas. Compilez avec: make v3"
    exit 1
fi

echo "✅ Binaires trouvés"
echo ""

# Test 1: Vérifier que les tailles sont différentes
size_v2=$(stat -c%s versions/v2_build/chess_engine_v2)
size_v3=$(stat -c%s versions/v3_build/chess_engine_v3)

echo "📊 Tailles des binaires:"
echo "   V2: $size_v2 bytes"
echo "   V3: $size_v3 bytes"

if [ "$size_v2" -eq "$size_v3" ]; then
    echo "   ⚠️  ATTENTION: Les tailles sont identiques!"
else
    echo "   ✅ Les tailles sont différentes (bon signe)"
fi
echo ""

# Test 2: Benchmark rapide à depth 5
echo "🎯 Benchmark rapide (depth 5):"
echo ""

echo "V2 (sans TT):"
v2_result=$(timeout 10 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v2_build/chess_engine_v2" 2>&1 | grep "^info depth 5" | tail -1)
v2_nodes=$(echo "$v2_result" | grep -oP 'nodes \K\d+')
v2_time=$(echo "$v2_result" | grep -oP 'time \K\d+')
echo "   Nodes: $v2_nodes"
echo "   Time:  ${v2_time}ms"
echo ""

echo "V3 (avec TT):"
v3_result=$(timeout 10 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v3_build/chess_engine_v3" 2>&1 | grep "^info depth 5" | tail -1)
v3_nodes=$(echo "$v3_result" | grep -oP 'nodes \K\d+')
v3_time=$(echo "$v3_result" | grep -oP 'time \K\d+')
echo "   Nodes: $v3_nodes"
echo "   Time:  ${v3_time}ms"
echo ""

# Calculer l'amélioration
if [ -n "$v2_nodes" ] && [ -n "$v3_nodes" ] && [ "$v2_nodes" -gt 0 ]; then
    improvement=$((100 - (v3_nodes * 100 / v2_nodes)))
    speedup=$((v2_time * 100 / v3_time))
    
    echo "📈 Résultats:"
    echo "   Réduction de nodes: -${improvement}%"
    echo "   Accélération: ${speedup}%"
    echo ""
    
    if [ "$improvement" -lt 15 ]; then
        echo "❌ PROBLÈME: V3 devrait explorer 20-30% de nœuds en moins que V2"
        echo "   Le fix n'a peut-être pas été appliqué correctement."
        exit 1
    else
        echo "✅ SUCCÈS: V3 explore significativement moins de nœuds que V2"
        echo "   Le Transposition Table fonctionne correctement!"
    fi
else
    echo "⚠️  Impossible de calculer l'amélioration (timeout ou erreur)"
fi

echo ""
echo "=========================================="
echo "Commande Fastchess Recommandée:"
echo "=========================================="
echo ""
echo "fastchess \\"
echo "  -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \\"
echo "  -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \\"
echo "  -each tc=10+0.1 \\"
echo "  -rounds 100 \\"
echo "  -repeat \\"
echo "  -concurrency 2 \\"
echo "  -pgn v2_vs_v3.pgn"
echo ""
echo "Résultat attendu: V3 devrait avoir ~60-70% de win rate"
echo ""

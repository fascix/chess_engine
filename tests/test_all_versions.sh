#!/bin/bash

# Script de test pour vérifier toutes les versions V1-V10

echo "=========================================="
echo "Validation de Toutes les Versions (V1-V10)"
echo "=========================================="
echo ""

# Vérifier que toutes les versions existent
missing=0
for v in {1..10}; do
    if [ ! -f "versions/v${v}_build/chess_engine_v${v}" ]; then
        echo "❌ V${v} manquant"
        missing=1
    fi
done

if [ $missing -eq 1 ]; then
    echo ""
    echo "Compilez les versions manquantes avec: make all_versions"
    exit 1
fi

echo "✅ Toutes les versions (V1-V10) sont présentes"
echo ""

# Afficher les tailles
echo "📊 Tailles des binaires:"
for v in {1..10}; do
    size=$(stat -c%s versions/v${v}_build/chess_engine_v${v})
    printf "   V%-2d: %6d bytes\n" $v $size
done
echo ""

# Benchmark de performance (depth 5)
echo "🎯 Benchmark de Performance (depth 5):"
echo ""
printf "%-8s %-10s %-10s %-10s %-15s\n" "Version" "Nodes" "Time(ms)" "NPS" "Amélioration"
echo "----------------------------------------------------------------"

v2_nodes=""
for v in {1..10}; do
    result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v${v}_build/chess_engine_v${v}" 2>&1 | grep "^info depth 5" | tail -1)
    nodes=$(echo "$result" | grep -oP 'nodes \K\d+')
    time=$(echo "$result" | grep -oP 'time \K\d+')
    nps=$(echo "$result" | grep -oP 'nps \K\d+')
    
    if [ -z "$nodes" ]; then
        printf "V%-7d %-10s %-10s %-10s %-15s\n" "$v" "timeout" "-" "-" "-"
    else
        # Calculer l'amélioration par rapport à V2
        improvement=""
        if [ $v -eq 2 ]; then
            v2_nodes=$nodes
            improvement="baseline"
        elif [ -n "$v2_nodes" ] && [ "$v2_nodes" -gt 0 ]; then
            reduction=$((100 - (nodes * 100 / v2_nodes)))
            improvement="-${reduction}%"
        fi
        
        printf "V%-7d %-10s %-10s %-10s %-15s\n" "$v" "$nodes" "$time" "$nps" "$improvement"
    fi
done
echo ""

# Vérifications clés
echo "🔍 Vérifications Clés:"
echo ""

# V3 vs V2
v2_result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v2_build/chess_engine_v2" 2>&1 | grep "^info depth 5" | tail -1)
v3_result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v3_build/chess_engine_v3" 2>&1 | grep "^info depth 5" | tail -1)

v2_nodes=$(echo "$v2_result" | grep -oP 'nodes \K\d+')
v3_nodes=$(echo "$v3_result" | grep -oP 'nodes \K\d+')

if [ -n "$v2_nodes" ] && [ -n "$v3_nodes" ] && [ "$v3_nodes" -lt "$v2_nodes" ]; then
    improvement=$((100 - (v3_nodes * 100 / v2_nodes)))
    echo "✅ V3 (TT) explore ${improvement}% de nœuds en moins que V2"
else
    echo "❌ V3 devrait explorer moins de nœuds que V2!"
fi

# V7 vs V6 (LMR devrait avoir un gros impact)
v6_result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v6_build/chess_engine_v6" 2>&1 | grep "^info depth 5" | tail -1)
v7_result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v7_build/chess_engine_v7" 2>&1 | grep "^info depth 5" | tail -1)

v6_nodes=$(echo "$v6_result" | grep -oP 'nodes \K\d+')
v7_nodes=$(echo "$v7_result" | grep -oP 'nodes \K\d+')

if [ -n "$v6_nodes" ] && [ -n "$v7_nodes" ] && [ "$v7_nodes" -lt "$v6_nodes" ]; then
    improvement=$((100 - (v7_nodes * 100 / v6_nodes)))
    echo "✅ V7 (LMR) explore ${improvement}% de nœuds en moins que V6"
else
    echo "❌ V7 devrait explorer beaucoup moins de nœuds que V6!"
fi

# V10 vs V2 (amélioration globale)
v10_result=$(timeout 15 bash -c "echo -e 'position startpos\ngo depth 5\nquit' | versions/v10_build/chess_engine_v10" 2>&1 | grep "^info depth 5" | tail -1)
v10_nodes=$(echo "$v10_result" | grep -oP 'nodes \K\d+')

if [ -n "$v2_nodes" ] && [ -n "$v10_nodes" ] && [ "$v10_nodes" -lt "$v2_nodes" ]; then
    improvement=$((100 - (v10_nodes * 100 / v2_nodes)))
    speedup=$((v2_nodes / v10_nodes))
    echo "✅ V10 explore ${improvement}% de nœuds en moins que V2 (${speedup}x plus efficace)"
else
    echo "⚠️  V10 devrait explorer beaucoup moins de nœuds que V2"
fi

echo ""
echo "=========================================="
echo "📋 Résumé"
echo "=========================================="
echo ""
echo "Si tous les tests passent ✅:"
echo "  → Toutes les versions sont correctement compilées"
echo "  → Les features progressives fonctionnent"
echo "  → Prêt pour les tests ELO avec fastchess"
echo ""
echo "Commandes fastchess recommandées:"
echo ""
echo "# Test V2 vs V3 (TT):"
echo "fastchess -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \\"
echo "          -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \\"
echo "          -each tc=10+0.1 -rounds 100 -pgn v2_vs_v3.pgn"
echo ""
echo "# Test Gauntlet (V10 contre tous):"
echo "fastchess -engine cmd=versions/v10_build/chess_engine_v10 name=v10 \\"
echo "          -engine cmd=versions/v1_build/chess_engine_v1 name=v1 \\"
echo "          -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \\"
echo "          -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \\"
echo "          -each tc=10+0.1 -rounds 50 -gauntlet -pgn gauntlet.pgn"
echo ""

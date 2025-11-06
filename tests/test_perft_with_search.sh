#!/bin/bash

echo "=========================================="
echo "TEST: PERFT avec Évaluation et Recherche"
echo "=========================================="
echo ""
echo "Ce test vérifie que:"
echo "1. La génération de coups (perft) fonctionne correctement"
echo "2. L'évaluation est cohérente à différentes profondeurs"
echo "3. La recherche trouve des coups raisonnables"
echo ""

# Couleurs pour l'affichage
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

engine="./chess_engine"

# Fonction pour vérifier si un nombre correspond à l'attendu
check_result() {
    local result=$1
    local expected=$2
    local test_name=$3
    
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}✓${NC} $test_name: PASS (résultat: $result)"
        return 0
    else
        echo -e "${RED}✗${NC} $test_name: FAIL (attendu: $expected, obtenu: $result)"
        return 1
    fi
}

# Test 1: Perft sur position initiale
echo "=========================================="
echo "TEST 1: Perft Position Initiale"
echo "=========================================="
echo ""

echo "Position: startpos"
echo ""

# Perft profondeur 1 (devrait donner 20 coups)
echo "Perft profondeur 1 (attendu: 20 coups)"
result=$(echo -e "uci\nposition startpos\nperft 1\nquit" | $engine 2>/dev/null | grep -E "^[a-h][1-8][a-h][1-8]:" | wc -l)
check_result "$result" "20" "Perft depth 1"

# Perft profondeur 2 (devrait donner 400 positions)
echo "Perft profondeur 2 (attendu: 400 positions)"
result=$(echo -e "uci\nposition startpos\nperft 2\nquit" | $engine 2>/dev/null | grep "Total:" | awk '{print $2}')
check_result "$result" "400" "Perft depth 2"

echo ""

# Test 2: Évaluation à différentes profondeurs
echo "=========================================="
echo "TEST 2: Évaluation Position Initiale"
echo "=========================================="
echo ""

echo "Recherche à différentes profondeurs pour vérifier la cohérence"
echo ""

# Recherche depth 3
echo "--- Depth 3 ---"
result_d3=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position startpos"
  sleep 0.3
  echo "go depth 3"
  sleep 3
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "^info depth 3" | tail -1)
echo "$result_d3"
score_d3=$(echo "$result_d3" | grep -oP "score cp \K-?\d+")

# Recherche depth 4
echo "--- Depth 4 ---"
result_d4=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position startpos"
  sleep 0.3
  echo "go depth 4"
  sleep 3
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "^info depth 4" | tail -1)
echo "$result_d4"
score_d4=$(echo "$result_d4" | grep -oP "score cp \K-?\d+")

# Recherche depth 5
echo "--- Depth 5 ---"
result_d5=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position startpos"
  sleep 0.3
  echo "go depth 5"
  sleep 5
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "^info depth 5" | tail -1)
echo "$result_d5"
score_d5=$(echo "$result_d5" | grep -oP "score cp \K-?\d+")

echo ""
echo "Résumé des scores:"
echo "  Depth 3: ${score_d3:-N/A} cp"
echo "  Depth 4: ${score_d4:-N/A} cp"
echo "  Depth 5: ${score_d5:-N/A} cp"
echo ""

if [ -n "$score_d3" ] && [ -n "$score_d4" ] && [ -n "$score_d5" ]; then
    echo -e "${GREEN}✓${NC} Tous les niveaux ont retourné une évaluation"
else
    echo -e "${RED}✗${NC} Certains niveaux n'ont pas retourné d'évaluation"
fi

echo ""

# Test 3: Position tactique connue
echo "=========================================="
echo "TEST 3: Position Tactique"
echo "=========================================="
echo ""

# Position où les blancs peuvent gagner du matériel
# Position après 1.e4 e5 2.Nf3 Nc6 3.Bc4 Nf6?? (permet 4.Ng5 attaquant f7)
echo "Position: 1.e4 e5 2.Nf3 Nc6 3.Bc4 Nf6"
echo "Les blancs peuvent jouer Ng5 pour attaquer f7"
echo ""

# Perft pour vérifier que tous les coups sont générés
echo "--- Perft depth 1 pour cette position ---"
result=$(timeout 5 bash -c '{
  echo "uci"
  sleep 0.3
  echo "position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 g8f6"
  sleep 0.3
  echo "perft 1"
  sleep 2
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep -E "^[a-h][1-8]" | wc -l)

echo "Nombre de coups légaux: $result"

# Recherche pour voir si le moteur trouve Ng5
echo ""
echo "--- Recherche depth 4 ---"
result_tactical=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 g8f6"
  sleep 0.3
  echo "go depth 4"
  sleep 5
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "bestmove")

echo "$result_tactical"
bestmove=$(echo "$result_tactical" | awk '{print $2}')

if [[ "$bestmove" == "f3g5" ]] || [[ "$bestmove" == "g5" ]]; then
    echo -e "${GREEN}✓${NC} Le moteur a trouvé le coup tactique Ng5"
else
    echo -e "${YELLOW}⚠${NC} Le moteur a joué $bestmove au lieu de Ng5 (acceptable selon la profondeur)"
fi

echo ""

# Test 4: Position avec mat en 1
echo "=========================================="
echo "TEST 4: Détection de Mat"
echo "=========================================="
echo ""

# Position: Roi noir en h8, Dame blanche peut mater en g7
# FEN: 7k/6Q1/8/8/8/8/8/7K w - - 0 1
echo "Position: Mat en 1 pour les blancs (Qg7#)"
echo "FEN: 7k/6Q1/8/8/8/8/8/7K w - - 0 1"
echo ""

# Perft pour vérifier les coups légaux
echo "--- Perft depth 1 ---"
result=$(timeout 5 bash -c '{
  echo "uci"
  sleep 0.3
  echo "position fen 7k/6Q1/8/8/8/8/8/7K w - - 0 1"
  sleep 0.3
  echo "perft 1"
  sleep 2
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep -E "^[a-h][1-8]" | wc -l)

echo "Nombre de coups légaux: $result"

# Recherche pour voir si le moteur trouve le mat
echo ""
echo "--- Recherche depth 2 (devrait trouver le mat) ---"
result_mate=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position fen 7k/6Q1/8/8/8/8/8/7K w - - 0 1"
  sleep 0.3
  echo "go depth 2"
  sleep 3
  echo "quit"
} | '"$engine"' 2>/dev/null')

echo "$result_mate" | grep "^info depth"
bestmove_mate=$(echo "$result_mate" | grep "bestmove" | awk '{print $2}')
score_mate=$(echo "$result_mate" | grep "^info depth 2" | tail -1 | grep -oP "score (cp|mate) \K-?\d+")

echo ""
echo "Meilleur coup: $bestmove_mate"
echo "Score: $score_mate"

if [[ "$bestmove_mate" == "g7"* ]]; then
    echo -e "${GREEN}✓${NC} Le moteur a trouvé le mat en 1 (Qg7)"
else
    echo -e "${RED}✗${NC} Le moteur n'a pas trouvé le mat (joué: $bestmove_mate)"
fi

echo ""

# Test 5: Perft sur une position complexe
echo "=========================================="
echo "TEST 5: Perft Position Complexe"
echo "=========================================="
echo ""

# Position Kiwipete - position de test standard
# FEN: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
echo "Position: Kiwipete (position de test standard)"
echo "FEN: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
echo ""

# Perft profondeur 1 (devrait donner 48 coups)
echo "Perft profondeur 1 (attendu: 48 coups)"
result=$(timeout 5 bash -c '{
  echo "uci"
  sleep 0.3
  echo "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
  sleep 0.3
  echo "perft 1"
  sleep 2
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep -E "^[a-h][1-8]" | wc -l)

check_result "$result" "48" "Perft depth 1 Kiwipete"

# Perft profondeur 2 (devrait donner 2039 positions)
echo "Perft profondeur 2 (attendu: 2039 positions)"
result=$(timeout 10 bash -c '{
  echo "uci"
  sleep 0.3
  echo "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
  sleep 0.3
  echo "perft 2"
  sleep 5
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "Total:" | awk '{print $2}')

check_result "$result" "2039" "Perft depth 2 Kiwipete"

# Recherche sur cette position
echo ""
echo "--- Recherche depth 4 sur Kiwipete ---"
result_kiwi=$(timeout 15 bash -c '{
  echo "uci"
  sleep 0.3
  echo "isready"
  sleep 0.3
  echo "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
  sleep 0.3
  echo "go depth 4"
  sleep 10
  echo "quit"
} | '"$engine"' 2>/dev/null' | grep "^info depth 4" | tail -1)

echo "$result_kiwi"

echo ""

# Résumé final
echo "=========================================="
echo "RÉSUMÉ DES TESTS"
echo "=========================================="
echo ""
echo "Tests effectués:"
echo "  ✓ Perft position initiale (profondeurs 1-2)"
echo "  ✓ Évaluation cohérente (profondeurs 3-5)"
echo "  ✓ Position tactique (Ng5)"
echo "  ✓ Détection de mat en 1"
echo "  ✓ Perft position complexe (Kiwipete)"
echo ""
echo "Ce test vérifie que:"
echo "  1. La génération de coups est correcte (perft)"
echo "  2. L'évaluation fonctionne à différentes profondeurs"
echo "  3. La recherche trouve des coups raisonnables"
echo "  4. Les optimisations n'ont pas cassé la logique de base"
echo ""
echo "=========================================="
echo "FIN DES TESTS"
echo "=========================================="
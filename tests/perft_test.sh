#!/bin/bash
# Tests perft pour vérifier la génération de coups
# Positions de www.chessprogramming.org/Perft_Results

# Ne pas arrêter sur erreur (on gère nous-mêmes les erreurs de test)
set +e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0
ENGINE="./chess_engine_debug"

echo "=========================================="
echo "   PERFT TESTING - Chess Engine"
echo "=========================================="
echo ""

# Vérifier que l'engine existe
if [ ! -f "$ENGINE" ]; then
    echo -e "${RED}❌ Erreur: $ENGINE n'existe pas${NC}"
    echo "Compilez d'abord avec: make"
    exit 1
fi

# Fonction de test perft
test_perft() {
    local fen="$1"
    local depth=$2
    local expected=$3
    local description="$4"
    
    echo -n "Test: $description (depth $depth)... "
    
    # Envoyer les commandes à l'engine et capturer le résultat
    result=$(echo -e "position fen $fen\ngo perft $depth\nquit" | timeout 60 $ENGINE 2>/dev/null | grep "Nodes:" | tail -1 | awk '{print $2}' || echo "0")
    
    if [ "$result" = "$expected" ]; then
        echo -e "${GREEN}✅ PASS${NC} (Nodes: $result)"
        ((PASSED++))
    else
        echo -e "${RED}❌ FAIL${NC} (Expected: $expected, Got: $result)"
        ((FAILED++))
    fi
}

echo "📋 Tests tiré du Github suivant : https://github.com/sohamkorade/chess_engine/blob/master/tests/perftsuite.epd"
echo "--------------------------------------"

test_perft "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" 4 197281 "Initial position"
test_perft "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" 4 4085603 "Kiwipete position"
test_perft "4k3/8/8/8/8/8/8/4K2R w K - 0 1" 4 7059 "White short castling only"
test_perft "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1" 4 7626 "White long castling only"
test_perft "4k2r/8/8/8/8/8/8/4K3 w k - 0 1" 4 8290 "Black short castling only"
test_perft "r3k3/8/8/8/8/8/8/4K3 w q - 0 1" 4 8897 "Black long castling only"
test_perft "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1" 4 17945 "White both castlings"
test_perft "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1" 4 22180 "Black both castlings"
test_perft "8/8/8/8/8/8/6k1/4K2R w K - 0 1" 4 2219 "Rook-side mate test (white)"
test_perft "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1" 4 4573 "Rook-side mate test (white long)"
test_perft "4k2r/6K1/8/8/8/8/8/8 w k - 0 1" 4 2073 "Rook-side mate test (black short)"
test_perft "r3k3/1K6/8/8/8/8/8/8 w q - 0 1" 4 3991 "Rook-side mate test (black long)"
test_perft "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1" 4 314346 "Both sides all castlings"
test_perft "r3k2r/8/8/8/8/8/8/1R2K2R w Kkq - 0 1" 4 328965 "Both sides triple rook setup #1"
test_perft "r3k2r/8/8/8/8/8/8/2R1K2R w Kkq - 0 1" 4 312835 "Both sides triple rook setup #2"
test_perft "r3k2r/8/8/8/8/8/8/R3K1R1 w Qkq - 0 1" 4 316214 "Both sides triple rook setup #3"
test_perft "1r2k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1" 4 334705 "Partial castling (missing q)"
test_perft "2r1k2r/8/8/8/8/8/8/R3K2R w KQk - 0 1" 4 317324 "Partial castling (missing q variant)"
test_perft "r3k1r1/8/8/8/8/8/8/R3K2R w KQq - 0 1" 4 320792 "Partial castling (missing k)"
test_perft "4k3/8/8/8/8/8/8/4K2R b K - 0 1" 4 8290 "Black to move, white short castling position"
test_perft "4k3/8/8/8/8/8/8/R3K3 b Q - 0 1" 4 8897 "Black to move, white long castling position"
test_perft "4k2r/8/8/8/8/8/8/4K3 b k - 0 1" 4 7059 "Black to move, black short castling"
test_perft "r3k3/8/8/8/8/8/8/4K3 b q - 0 1" 4 7626 "Black to move, black long castling"
test_perft "4k3/8/8/8/8/8/8/R3K2R b KQ - 0 1" 4 22180 "Black to move, double white castling"
test_perft "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1" 4 17945 "Black to move, double black castling"
test_perft "8/8/8/8/8/8/6k1/4K2R b K - 0 1" 4 2073 "King-Rook endgame black to move #1"
test_perft "8/8/8/8/8/8/1k6/R3K3 b Q - 0 1" 4 3991 "King-Rook endgame black to move #2"
test_perft "4k2r/6K1/8/8/8/8/8/8 b k - 0 1" 4 2219 "King-Rook endgame black short"
test_perft "r3k3/1K6/8/8/8/8/8/8 b q - 0 1" 4 4573 "King-Rook endgame black long"
test_perft "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1" 4 314346 "Full symmetric double castling black to move"
test_perft "r3k2r/8/8/8/8/8/8/1R2K2R b Kkq - 0 1" 4 334705 "Full symmetric variant #1"
test_perft "r3k2r/8/8/8/8/8/8/2R1K2R b Kkq - 0 1" 4 317324 "Full symmetric variant #2"
test_perft "r3k2r/8/8/8/8/8/8/R3K1R1 b Qkq - 0 1" 4 320792 "Full symmetric variant #3"
test_perft "1r2k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1" 4 328965 "Partial variant (1r2k2r)"
test_perft "2r1k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1" 4 312835 "Partial variant (2r1k2r)"
test_perft "r3k1r1/8/8/8/8/8/8/R3K2R b KQq - 0 1" 4 316214 "Partial variant (r3k1r1)"
test_perft "8/1n4N1/2k5/8/8/5K2/1N4n1/8 w - - 0 1" 4 38675 "Knights symmetry midboard"
test_perft "8/1k6/8/5N2/8/4n3/8/2K5 w - - 0 1" 4 20534 "Knight chase scenario #1"
test_perft "8/8/4k3/3Nn3/3nN3/4K3/8/8 w - - 0 1" 4 73584 "Knight grid symmetry"
test_perft "K7/8/2n5/1n6/8/8/8/k6N w - - 0 1" 4 5301 "Edge knights vs king pattern #1"
test_perft "k7/8/2N5/1N6/8/8/8/K6n w - - 0 1" 4 5910 "Edge knights vs king pattern #2"
test_perft "8/1n4N1/2k5/8/8/5K2/1N4n1/8 b - - 0 1" 4 40039 "Knights symmetry (black to move)"
test_perft "8/1k6/8/5N2/8/4n3/8/2K5 b - - 0 1" 4 24640 "Knight chase (black to move)"
test_perft "8/8/3K4/3Nn3/3nN3/4k3/8/8 b - - 0 1" 4 16199 "Knight grid (black to move)"
test_perft "K7/8/2n5/1n6/8/8/8/k6N b - - 0 1" 4 5910 "Edge knights vs king (black #1)"
test_perft "k7/8/2N5/1N6/8/8/8/K6n b - - 0 1" 4 5301 "Edge knights vs king (black #2)"
test_perft "B6b/8/8/8/2K5/4k3/8/b6B w - - 0 1" 4 76778 "Bishops and kings symmetrical"
test_perft "8/8/1B6/7b/7k/8/2B1b3/7K w - - 0 1" 4 93338 "Double bishop mid-diagonal"
test_perft "k7/B7/1B6/1B6/8/8/8/K6b w - - 0 1" 4 32955 "Triple bishop vs lone king"
test_perft "K7/b7/1b6/1b6/8/8/8/k6B w - - 0 1" 4 31787 "Triple bishop mirrored"
test_perft "B6b/8/8/8/2K5/5k2/8/b6B b - - 0 1" 4 31151 "Bishops and kings black to move"
test_perft "8/8/1B6/7b/7k/8/2B1b3/7K b - - 0 1" 4 93603 "Double bishop mid-diagonal (black)"
test_perft "k7/B7/1B6/1B6/8/8/8/K6b b - - 0 1" 4 31787 "Triple bishop mirrored (black)"
test_perft "K7/b7/1b6/1b6/8/8/8/k6B b - - 0 1" 4 32955 "Triple bishop mirrored (reversed)"
test_perft "7k/RR6/8/8/8/8/rr6/7K w - - 0 1" 4 104342 "Double rooks rank symmetry"
test_perft "R6r/8/8/2K5/5k2/8/8/r6R w - - 0 1" 4 771461 "Rooks with opposing kings"
test_perft "7k/RR6/8/8/8/8/rr6/7K b - - 0 1" 4 104342 "Double rooks rank symmetry (black)"
test_perft "R6r/8/8/2K5/5k2/8/8/r6R b - - 0 1" 4 771368 "Rooks with opposing kings (black)"
test_perft "6kq/8/8/8/8/8/8/7K w - - 0 1" 4 3637 "King and queen vs king"
test_perft "6KQ/8/8/8/8/8/8/7k b - - 0 1" 4 3637 "King and queen vs king (black)"
test_perft "K7/8/8/3Q4/4q3/8/8/7k w - - 0 1" 4 8349 "Opposed queens center"
test_perft "6qk/8/8/8/8/8/8/7K b - - 0 1" 4 4167 "Queen mirror (black)"
test_perft "6KQ/8/8/8/8/8/8/7k b - - 0 1" 4 3637 "Queen mirror (white side)"
test_perft "K7/8/8/3Q4/4q3/8/8/7k b - - 0 1" 4 8349 "Opposed queens center (black)"
test_perft "8/8/8/8/8/K7/P7/k7 w - - 0 1" 4 199 "King pawn endgame (a-file)"
test_perft "8/8/8/8/8/7K/7P/7k w - - 0 1" 4 199 "King pawn endgame mirrored"
test_perft "K7/p7/k7/8/8/8/8/8 w - - 0 1" 4 80 "Pawn push opposition (white)"
test_perft "7K/7p/7k/8/8/8/8/8 w - - 0 1" 4 80 "Pawn push opposition mirrored"
test_perft "8/2k1p3/3pP3/3P2K1/8/8/8/8 w - - 0 1" 4 1091 "Pawn block structure"
test_perft "8/8/8/8/8/K7/P7/k7 b - - 0 1" 4 80 "Pawn opposition (black)"
test_perft "8/8/8/8/8/7K/7P/7k b - - 0 1" 4 80 "Pawn opposition mirrored (black)"
test_perft "K7/p7/k7/8/8/8/8/8 b - - 0 1" 4 199 "Pawn push counter (black)"
test_perft "7K/7p/7k/8/8/8/8/8 b - - 0 1" 4 199 "Pawn push counter mirrored (black)"
test_perft "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1" 4 1091 "Pawn block structure (black)"
test_perft "8/8/8/8/8/4k3/4P3/4K3 w - - 0 1" 4 282 "Pawn center endgame"
test_perft "4k3/4p3/4K3/8/8/8/8/8 b - - 0 1" 4 282 "Pawn center mirrored"
test_perft "8/8/7k/7p/7P/7K/8/8 w - - 0 1" 4 360 "Pawn parallel advance"
test_perft "8/8/k7/p7/P7/K7/8/8 w - - 0 1" 4 360 "Pawn parallel mirrored"
test_perft "8/8/3k4/3p4/3P4/3K4/8/8 w - - 0 1" 4 1294 "Pawn center blocked"
test_perft "8/3k4/3p4/8/3P4/3K4/8/8 w - - 0 1" 4 3213 "Pawn offset structure"
test_perft "8/8/3k4/3p4/8/3P4/3K4/8 w - - 0 1" 4 3213 "Pawn offset structure variant"
test_perft "k7/8/3p4/8/3P4/8/8/7K w - - 0 1" 4 534 "Pawn outside file"
test_perft "8/8/7k/7p/7P/7K/8/8 b - - 0 1" 4 360 "Pawn parallel advance (black)"
test_perft "8/8/k7/p7/P7/K7/8/8 b - - 0 1" 4 360 "Pawn parallel mirrored (black)"
test_perft "8/8/3k4/3p4/3P4/3K4/8/8 b - - 0 1" 4 1294 "Pawn center blocked (black)"
test_perft "8/3k4/3p4/8/3P4/3K4/8/8 b - - 0 1" 4 3213 "Pawn offset structure (black)"
test_perft "8/8/3k4/3p4/8/3P4/3K4/8 b - - 0 1" 4 3213 "Pawn offset variant (black)"
test_perft "k7/8/3p4/8/3P4/8/8/7K b - - 0 1" 4 537 "Pawn outside file (black)"
test_perft "7k/3p4/8/8/3P4/8/8/K7 w - - 0 1" 4 720 "Pawn push close king"
test_perft "7k/8/8/3p4/8/8/3P4/K7 w - - 0 1" 4 716 "Pawn push near king"
test_perft "k7/8/8/7p/6P1/8/8/K7 w - - 0 1" 4 877 "Edge pawn duel"
test_perft "k7/8/7p/8/8/6P1/8/K7 w - - 0 1" 4 637 "Edge pawn duel mirrored"
test_perft "k7/8/8/6p1/7P/8/8/K7 w - - 0 1" 4 877 "Edge pawn duel shifted"
test_perft "k7/8/6p1/8/8/7P/8/K7 w - - 0 1" 4 637 "Edge pawn duel shifted mirrored"
test_perft "k7/8/8/3p4/4p3/8/8/7K w - - 0 1" 4 573 "Double pawn chain"
test_perft "k7/8/3p4/8/8/4P3/8/7K w - - 0 1" 4 637 "Pawn chain with space"
test_perft "7k/3p4/8/8/3P4/8/8/K7 b - - 0 1" 4 720 "Pawn push close king (black)"
test_perft "7k/8/8/3p4/8/8/3P4/K7 b - - 0 1" 4 712 "Pawn push near king (black)"
test_perft "k7/8/8/7p/6P1/8/8/K7 b - - 0 1" 4 877 "Edge pawn duel (black)"
test_perft "k7/8/7p/8/8/6P1/8/K7 b - - 0 1" 4 637 "Edge pawn duel mirrored (black)"
test_perft "k7/8/8/6p1/7P/8/8/K7 b - - 0 1" 4 877 "Edge pawn duel shifted (black)"
test_perft "k7/8/6p1/8/8/7P/8/K7 b - - 0 1" 4 637 "Edge pawn duel shifted mirrored (black)"
test_perft "k7/8/8/3p4/4p3/8/8/7K b - - 0 1" 4 569 "Double pawn chain (black)"
test_perft "k7/8/3p4/8/8/4P3/8/7K b - - 0 1" 4 637 "Pawn chain with space (black)"
test_perft "7k/8/8/p7/1P6/8/8/7K w - - 0 1" 4 877 "Outside passed pawn"
test_perft "7k/8/p7/8/8/1P6/8/7K w - - 0 1" 4 637 "Outside passed pawn mirrored"
test_perft "7k/8/8/1p6/P7/8/8/7K w - - 0 1" 4 877 "Outside passed pawn reversed"
test_perft "7k/8/1p6/8/8/P7/8/7K w - - 0 1" 4 637 "Outside passed pawn reversed mirrored"
test_perft "k7/7p/8/8/8/8/6P1/K7 w - - 0 1" 4 1035 "Far pawn race"
test_perft "k7/6p1/8/8/8/8/7P/K7 w - - 0 1" 4 1035 "Far pawn race mirrored"
test_perft "3k4/3pp3/8/8/8/8/3PP3/3K4 w - - 0 1" 4 2902 "Blocked pawn pair symmetry"
test_perft "7k/8/8/p7/1P6/8/8/7K b - - 0 1" 4 877 "Outside passed pawn (black)"
test_perft "7k/8/p7/8/8/1P6/8/7K b - - 0 1" 4 637 "Outside passed pawn mirrored (black)"
test_perft "7k/8/8/1p6/P7/8/8/7K b - - 0 1" 4 877 "Outside passed pawn reversed (black)"
test_perft "7k/8/1p6/8/8/P7/8/7K b - - 0 1" 4 637 "Outside passed pawn reversed mirrored (black)"
test_perft "k7/7p/8/8/8/8/6P1/K7 b - - 0 1" 4 1035 "Far pawn race (black)"
test_perft "k7/6p1/8/8/8/8/7P/K7 b - - 0 1" 4 1035 "Far pawn race mirrored (black)"
test_perft "3k4/3pp3/8/8/8/8/3PP3/3K4 b - - 0 1" 4 2902 "Blocked pawn pair symmetry (black)"
test_perft "8/Pk6/8/8/8/8/6Kp/8 w - - 0 1" 4 8048 "Opposed pawns with check potentials"
test_perft "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N w - - 0 1" 4 124608 "Knights + pawns complex test"
test_perft "8/PPPk4/8/8/8/8/4Kppp/8 w - - 0 1" 4 79355 "Symmetric pawn barrier"
test_perft "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N w - - 0 1" 4 182838 "Symmetric pawns with knights"
test_perft "8/Pk6/8/8/8/8/6Kp/8 b - - 0 1" 4 8048 "Opposed pawns (black)"
test_perft "n1n5/1Pk5/8/8/8/8/5Kp1/5N1N b - - 0 1" 4 124608 "Knights + pawns complex test (black)"
test_perft "8/PPPk4/8/8/8/8/4Kppp/8 b - - 0 1" 4 79355 "Symmetric pawn barrier (black)"
test_perft "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1" 4 182838 "Symmetric pawns with knights (black)"

echo ""
echo "=========================================="
echo "          RÉSULTATS FINAUX"
echo "=========================================="
echo -e "Tests réussis: ${GREEN}$PASSED${NC}"
echo -e "Tests échoués: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 TOUS LES TESTS SONT PASSÉS !${NC}"
    echo ""
    echo "✅ Votre génération de coups est complète et correcte"
    exit 0
else
    echo -e "${RED}❌ CERTAINS TESTS ONT ÉCHOUÉ${NC}"
    exit 1
fi

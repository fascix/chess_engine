#!/bin/bash
# Script principal pour lancer tous les tests du moteur d'échecs

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "╔════════════════════════════════════════════════╗"
echo "║      SUITE DE TESTS COMPLÈTE - Pallas v2.0     ║"
echo "╚════════════════════════════════════════════════╝"
echo

# Compteur de tests
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Fonction pour exécuter un test
run_test() {
    local test_name="$1"
    local test_script="$2"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📋 Test #$TOTAL_TESTS: $test_name"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if bash "$SCRIPT_DIR/$test_script"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo "✅ $test_name: RÉUSSI"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo "❌ $test_name: ÉCHOUÉ"
    fi
    echo
}

# Vérifier que le moteur est compilé
if [ ! -f "$PROJECT_DIR/pallas" ]; then
    echo "❌ ERREUR: Le moteur n'est pas compilé. Exécutez 'make' d'abord."
    exit 1
fi

# Exécuter les tests
echo "🚀 Lancement des tests..."
echo

# Test 1: Conformité UCI
run_test "Conformité UCI" "uci_compliance_test.sh"

# Test 2: Perft (génération de coups)
# Note: Les tests perft valident aussi indirectement le Zobrist incrémental
# car si le hash était cassé, les résultats seraient incorrects
run_test "Perft (génération de coups)" "perft_test.sh"
# Résumé final
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 RÉSUMÉ DES TESTS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Total de tests:  $TOTAL_TESTS"
echo "Tests réussis:   $PASSED_TESTS ✅"
echo "Tests échoués:   $FAILED_TESTS ❌"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ $FAILED_TESTS -eq 0 ]; then
    echo
    echo "🎉 TOUS LES TESTS SONT PASSÉS!"
    echo
    exit 0
else
    echo
    echo "⚠️  $FAILED_TESTS test(s) ont échoué"
    echo
    exit 1
fi
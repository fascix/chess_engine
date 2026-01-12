#!/bin/bash
# Test de validation du Zobrist incrémental
# Ce test vérifie que le hash incrémental est identique au hash recalculé

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
TEST_BINARY="$PROJECT_DIR/test_zobrist"
TEST_SOURCE="$PROJECT_DIR/tests/zobrist_incremental_test.c"

echo "=== TEST ZOBRIST INCRÉMENTAL ==="
echo

# Vérifier que les fichiers build existent
if [ ! -d "$PROJECT_DIR/build" ]; then
    echo "❌ ERREUR: Le dossier build n'existe pas. Compilez d'abord le projet avec 'make'."
    exit 1
fi

# Compiler le test
echo "📦 Compilation du test..."
gcc -Wall -Wextra -std=c11 -I"$PROJECT_DIR/Engine" -O2 \
    -o "$TEST_BINARY" "$TEST_SOURCE" \
    "$PROJECT_DIR/build/board.o" \
    "$PROJECT_DIR/build/movegen.o" \
    "$PROJECT_DIR/build/zobrist.o" \
    "$PROJECT_DIR/build/utils.o" \
    -lm

if [ $? -ne 0 ]; then
    echo "❌ ERREUR: Échec de la compilation"
    exit 1
fi

echo "✅ Compilation réussie"
echo

# Exécuter le test
echo "🧪 Exécution des tests..."
echo
"$TEST_BINARY"

TEST_RESULT=$?

# Nettoyer le binaire de test
rm -f "$TEST_BINARY"

if [ $TEST_RESULT -eq 0 ]; then
    echo
    echo "✅ TOUS LES TESTS ZOBRIST PASSENT"
    exit 0
else
    echo
    echo "❌ ÉCHEC DES TESTS ZOBRIST"
    exit 1
fi

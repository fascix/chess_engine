# =============================================================================
# 🏛️  PALLAS CHESS ENGINE - Makefile
# =============================================================================
# Ce Makefile permet de compiler le moteur Pallas dans différentes versions
# (complète, sans livre, sans tablebases, ou pure).

CC = gcc

# -----------------------------------------------------------------------------
# OPTIONS DE COMPILATION
# -----------------------------------------------------------------------------

CFLAGS_COMMON = -Wall -Wextra -std=c11 -IEngine
CFLAGS_DEBUG = -g -DDEBUG -fsanitize=address,undefined
CFLAGS_RELEASE = -O3 -march=native -flto -DNDEBUG

# Bibliothèques à lier (maths)
LIBS = -lm

# -----------------------------------------------------------------------------
# DOSSIERS ET FICHIERS
# -----------------------------------------------------------------------------

BUILD_DIR = build
BUILD_DIR_DEBUG = build_debug

# Modules du moteur (logique métier)
MODULES = Engine/board.c \
          Engine/movegen.c \
          Engine/utils.c \
          Engine/evaluation.c \
          Engine/zobrist.c \
          Engine/transposition.c \
          Engine/move_ordering.c \
          Engine/quiescence.c \
          Engine/search_helpers.c \
          Engine/logger.c \
          Engine/vendor/log.c \
          Engine/polyglot.c \
          Engine/syzygy.c

# Sources principales (UCI et Recherche)
SRC = $(MODULES) \
      Engine/perft.c \
      Engine/uci.c \
      Engine/timemanager.c \
      Engine/search.c \
      Engine/main.c

# Objets correspondants
OBJ_RELEASE = $(patsubst Engine/%.c,$(BUILD_DIR)/%.o,$(SRC))
OBJ_DEBUG = $(patsubst Engine/%.c,$(BUILD_DIR_DEBUG)/%.o,$(SRC))

# -----------------------------------------------------------------------------
# CIBLES PRINCIPALES
# -----------------------------------------------------------------------------

# Cible par défaut : compilation en mode release complète
all: pallas

# Compilation de l'exécutable principal optimisé
pallas: $(OBJ_RELEASE)
	@echo "🔗 Liaison de l'exécutable pallas..."
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o $@ $^ $(LIBS)
	@echo "✅ Pallas compilé avec succès."

# Compilation en mode debug avec sanitizers
debug: pallas-debug

pallas-debug: $(OBJ_DEBUG)
	@echo "🔗 Liaison de l'exécutable pallas-debug..."
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -o $@ $^ $(LIBS)
	@echo "✅ Pallas-debug compilé avec succès."

# -----------------------------------------------------------------------------
# VERSIONS SPÉCIFIQUES (DÉSACTIVATION DE FEATURES)
# -----------------------------------------------------------------------------

# Version sans support du livre d'ouverture (.bin)
pallas-no-book:
	@echo "🔨 Compilation de Pallas (sans opening book)..."
	@$(MAKE) clean-build
	@$(MAKE) CFLAGS_RELEASE="$(CFLAGS_RELEASE) -DDISABLE_BOOK" pallas
	@mv pallas pallas-no-book
	@echo "✅ pallas-no-book généré."

# Version sans support des tablebases Syzygy
pallas-no-tb:
	@echo "🔨 Compilation de Pallas (sans tablebases)..."
	@$(MAKE) clean-build
	@$(MAKE) CFLAGS_RELEASE="$(CFLAGS_RELEASE) -DDISABLE_TABLEBASES" pallas
	@mv pallas pallas-no-tb
	@echo "✅ pallas-no-tb généré."

# Version "Pure" : algorithme de recherche et évaluation uniquement
pallas-pure:
	@echo "🔨 Compilation de Pallas (Pure - sans book ni tablebases)..."
	@$(MAKE) clean-build
	@$(MAKE) CFLAGS_RELEASE="$(CFLAGS_RELEASE) -DDISABLE_BOOK -DDISABLE_TABLEBASES" pallas
	@mv pallas pallas-pure
	@echo "✅ pallas-pure généré."

# -----------------------------------------------------------------------------
# RÈGLES DE COMPILATION DES OBJETS
# -----------------------------------------------------------------------------

# Compilation des objets Release
$(BUILD_DIR)/%.o: Engine/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

# Compilation des objets Debug
$(BUILD_DIR_DEBUG)/%.o: Engine/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

# -----------------------------------------------------------------------------
# TESTS UNITAIRES (UNITY)
# -----------------------------------------------------------------------------

TESTS_DIR = tests
UNITY_DIR = $(TESTS_DIR)/unity
BUILD_TESTS_DIR = build_tests
UNITY_SRC = $(UNITY_DIR)/unity.c
TEST_SOURCES = $(wildcard $(TESTS_DIR)/test_*.c)
TEST_EXECUTABLES = $(patsubst $(TESTS_DIR)/test_%.c,$(BUILD_TESTS_DIR)/test_%,$(TEST_SOURCES))
CFLAGS_TEST = $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -I$(UNITY_DIR)

$(BUILD_TESTS_DIR)/test_%: $(TESTS_DIR)/test_%.c $(UNITY_SRC) $(MODULES)
	@mkdir -p $(BUILD_TESTS_DIR)
	@$(CC) $(CFLAGS_TEST) -o $@ $< $(UNITY_SRC) $(MODULES) $(LIBS)

# Compile tous les tests unitaires
build-tests: $(TEST_EXECUTABLES)
	@echo "✅ Tous les tests unitaires ont été compilés dans $(BUILD_TESTS_DIR)/"

# Exécute tous les tests unitaires via Unity
test: build-tests
	@echo "=========================================="
	@echo "   Exécution des tests unitaires Unity    "
	@echo "=========================================="
	@for test in $(TEST_EXECUTABLES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "✅ Tous les tests unitaires sont passés !"

# Exécute la suite complète de tests (UCI, Perft, Unity)
test-all: all build-tests
	@bash tests/run_all_tests.sh
	@$(MAKE) test

# -----------------------------------------------------------------------------
# NETTOYAGE
# -----------------------------------------------------------------------------

# Nettoie uniquement les fichiers objets
clean-build:
	@rm -rf $(BUILD_DIR) $(BUILD_DIR_DEBUG)

# Nettoie tout (objets et exécutables)
clean: clean-build
	@echo "🧹 Nettoyage complet des binaires..."
	@rm -rf $(BUILD_TESTS_DIR)
	@rm -f pallas pallas-debug pallas-no-book pallas-no-tb pallas-pure chess_engine
	@echo "✅ Nettoyage terminé."

# Nettoie les fichiers de logs
clean-logs:
	@echo "🧹 Nettoyage des fichiers de logs..."
	@rm -rf logs/*.log logs/*.txt
	@rm -rf fastchess-mac-arm64/logs/*
	@rm -rf fastchess-mac-arm64/pgn_results/*
	@echo "✅ Logs nettoyés."

# Nettoyage total
distclean: clean clean-logs

# -----------------------------------------------------------------------------
# AIDE
# -----------------------------------------------------------------------------

help:
	@echo "🏛️  PALLAS CHESS ENGINE - Guide de compilation"
	@echo ""
	@echo "Usage: make [cible]"
	@echo ""
	@echo "CIBLES DE COMPILATION :"
	@echo "  make              - Compile 'pallas' (version standard optimisée)"
	@echo "  make debug        - Compile 'pallas-debug' (avec sanitizers et symboles)"
	@echo "  make pallas-no-book - Compile une version sans support du livre d'ouverture"
	@echo "  make pallas-no-tb   - Compile une version sans support des tablebases"
	@echo "  make pallas-pure    - Compile une version 'pure' (sans book ni tablebases)"
	@echo ""
	@echo "CIBLES DE TEST :"
	@echo "  make test         - Exécute les tests unitaires Unity"
	@echo "  make test-all     - Exécute TOUS les tests (UCI, Perft, Unity)"
	@echo "  make build-tests  - Compile uniquement les tests unitaires"
	@echo ""
	@echo "CIBLES DE NETTOYAGE :"
	@echo "  make clean        - Supprime les objets et tous les binaires Pallas"
	@echo "  make clean-logs   - Supprime les logs du moteur et de fastchess"
	@echo "  make distclean    - Nettoyage total (builds + logs)"
	@echo ""
	@echo "AUTRES :"
	@echo "  make help         - Affiche ce message d'aide"

# Inclusion automatique des dépendances générées par -MMD
-include $(BUILD_DIR)/*.d
-include $(BUILD_DIR_DEBUG)/*.d

.PHONY: all debug pallas-debug pallas-no-book pallas-no-tb pallas-pure clean clean-build clean-logs clean-all test test-all build-tests help distclean

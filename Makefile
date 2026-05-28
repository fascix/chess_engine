# =============================================================================
# 🏛️  PALLAS CHESS ENGINE - Makefile
# =============================================================================
# Ce Makefile permet de compiler le moteur Pallas dans différentes versions
# (complète, sans livre, sans tablebases, ou pure).

CC = gcc

EMSDK_DIR = emsdk
# Chemin vers Emscripten SDK (pour la compilation WebAssembly)

CFLAGS_COMMON = -Wall -Wextra -std=c11 -IEngine
# Options communes de compilation :
# -Wall et -Wextra activent des warnings supplémentaires pour un code plus sûr
# -std=c11 spécifie la norme C utilisée
# -IEngine ajoute le dossier Engine aux chemins d'inclusion des headers

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
# Liste les fichiers objets pour la build debug, placés dans le dossier build_debug

# Cible par défaut : compilation en mode release
all: pallas

# Cible pour la compilation en mode debug
debug: pallas_debug

# Alias pour la compilation release
release: pallas

# Création du dossier build s'il n'existe pas, nécessaire pour y placer les fichiers objets release
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Création du dossier build_debug s'il n'existe pas, nécessaire pour y placer les fichiers objets debug
$(BUILD_DIR_DEBUG):
	mkdir -p $(BUILD_DIR_DEBUG)

# Règle de compilation des fichiers sources en mode release
# $< est le fichier source, $@ est le fichier cible
# -MMD -MP génèrent les fichiers de dépendances automatiques (.d)
$(BUILD_DIR)/%.o: Engine/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

# Compilation des objets Debug
$(BUILD_DIR_DEBUG)/%.o: Engine/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

# -----------------------------------------------------------------------------
# TESTS UNITAIRES (UNITY)
# -----------------------------------------------------------------------------
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

# Construction de l'exécutable de release à partir des fichiers objets correspondants
# -lm lie la bibliothèque mathématique
pallas: $(OBJ_RELEASE)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o $@ $^ -lm

# Construction de l'exécutable de debug à partir des fichiers objets correspondants
pallas_debug: $(OBJ_DEBUG)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -o $@ $^ -lm

# ========== COMPILATION WEBASSEMBLY (via Emscripten) ==========

EMCC = $(EMSDK_DIR)/upstream/emscripten/emcc
WASM_DIR = wasm

# Sources WASM (sans main.c, on utilise notre propre entry point)
SRC_WASM = $(MODULES_COMMON) Engine/perft.c Engine/uci.c Engine/timemanager.c Engine/search.c Engine/main.c

CFLAGS_WASM = -Wall -Wextra -std=c11 -IEngine -O3 -DNDEBUG \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s INITIAL_MEMORY=67108864 \
  -s TOTAL_STACK=2097152 \
  -s EXPORTED_FUNCTIONS='["_pallas_init","_pallas_uci_command","_pallas_reset","_pallas_get_legal_moves","_pallas_is_legal_move","_main","_malloc"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='PallasEngine' \
  -s ENVIRONMENT='worker' \
  -s SINGLE_FILE=1

wasm:
	@mkdir -p $(WASM_DIR)
	@echo "🌐 Compilation WebAssembly..."
	@echo "  Utilisation de: $(EMCC)"
	$(EMCC) $(CFLAGS_WASM) -o $(WASM_DIR)/pallas.js $(SRC_WASM) -lm
	@echo "✅ Compilation WASM terminée dans $(WASM_DIR)/"

wasm-clean:
	@echo "🧹 Nettoyage des fichiers WASM..."
	@rm -f $(WASM_DIR)/pallas.js
	@echo "✅ Fichiers WASM nettoyés"

# ========== CIBLES DE NETTOYAGE ==========

# Nettoyage basique : supprime les exécutables et dossiers build
clean:
	@echo "🧹 Nettoyage des builds principaux..."
	@rm -f pallas_debug
	@rm -rf $(BUILD_DIR) $(BUILD_DIR_DEBUG)
	@rm -rf pallas
	@echo "✅ Nettoyage terminé"

# Nettoyage des logs et fichiers temporaires
clean-logs:
	@echo "🧹 Nettoyage des logs..."
	@rm -rf logs/*.log logs/*.txt
	@rm -rf pgn_results/*.pgn
	@echo "✅ Logs nettoyés"

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
	@echo "  🌐 WEBASSEMBLY :"
	@echo "    make wasm             - Compile l'engine en WebAssembly"
	@echo "    make wasm-clean       - Nettoie les fichiers WASM"
	@echo ""
	@echo "  🧪 TESTS :"
	@echo "    make build-tests      - Compile les tests unitaires"
	@echo "    make test             - Compile et exécute tous les tests unitaires"
	@echo "    make clean-tests      - Nettoie les tests compilés"
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
# Déclaration des cibles "virtuelles" pour éviter des conflits avec des fichiers du même nom
.PHONY: all debug release clean clean-logs clean-all distclean rebuild help build-tests test clean-tests wasm wasm-clean

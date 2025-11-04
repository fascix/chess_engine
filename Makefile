CC = gcc
# Compilateur utilisé pour construire le projet, ici gcc (GNU Compiler Collection)

CFLAGS_COMMON = -Wall -Wextra -std=c11 -IEngine
# Options communes de compilation :
# -Wall et -Wextra activent des warnings supplémentaires pour un code plus sûr
# -std=c11 spécifie la norme C utilisée
# -IEngine ajoute le dossier Engine aux chemins d'inclusion des headers

CFLAGS_DEBUG = -g -DDEBUG -fsanitize=address,undefined
# Options spécifiques pour la compilation en mode debug :
# -g génère les informations de debug
# -DDEBUG définit la macro DEBUG pour activer le code de debug
# -fsanitize=address,undefined active les outils de détection d'erreurs mémoire et undefined behavior

CFLAGS_RELEASE = -O3 -march=native -flto -DNDEBUG
# Options spécifiques pour la compilation en mode release :
# -O3 active les optimisations agressives
# -march=native optimise pour l'architecture de la machine locale
# -flto active le Link Time Optimization pour des optimisations globales
# -DNDEBUG désactive les assertions (assert)

# Dossiers de build pour les fichiers objets (.o) et les fichiers de dépendances (.d)
BUILD_DIR = build
BUILD_DIR_DEBUG = build_debug
# Ces dossiers permettent de séparer les fichiers objets et dépendances selon le type de build (release ou debug)

# ========== MODULES COMMUNS ==========
MODULES_COMMON = Engine/board.c Engine/movegen.c Engine/utils.c Engine/evaluation.c \
                 Engine/zobrist.c Engine/transposition.c Engine/move_ordering.c \
                 Engine/quiescence.c Engine/search_helpers.c

# ========== SOURCES PRINCIPALES ==========
SRC = $(MODULES_COMMON) Engine/perft.c Engine/uci.c Engine/timemanager.c Engine/search.c Engine/main.c
# Liste tous les fichiers sources .c dans le dossier Engine
# Note: Liste explicite pour contrôler l'ordre de compilation

OBJ_RELEASE = $(patsubst Engine/%.c,$(BUILD_DIR)/%.o,$(SRC))
# Liste les fichiers objets pour la build release, placés dans le dossier build

OBJ_DEBUG = $(patsubst Engine/%.c,$(BUILD_DIR_DEBUG)/%.o,$(SRC))
# Liste les fichiers objets pour la build debug, placés dans le dossier build_debug

# Cible par défaut : compilation en mode release
all: chess_engine

# Cible pour la compilation en mode debug
debug: chess_engine_debug

# Alias pour la compilation release
release: chess_engine

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
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -MMD -MP -c $< -o $@

# Règle de compilation des fichiers sources en mode debug
# Même principe que pour release mais avec les options de debug
$(BUILD_DIR_DEBUG)/%.o: Engine/%.c | $(BUILD_DIR_DEBUG)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -MMD -MP -c $< -o $@

# Construction de l'exécutable de release à partir des fichiers objets correspondants
# -lm lie la bibliothèque mathématique
chess_engine: $(OBJ_RELEASE)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o $@ $^ -lm

# Construction de l'exécutable de debug à partir des fichiers objets correspondants
chess_engine_debug: $(OBJ_DEBUG)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -o $@ $^ -lm

# Nettoyage des fichiers générés : exécutables et dossiers de build
clean:
	rm -rf chess_engine chess_engine_debug $(BUILD_DIR) $(BUILD_DIR_DEBUG) versions/*/chess_engine_*

# Inclusion des fichiers de dépendances automatiques générés lors de la compilation
# Cela permet à make de connaître les dépendances exactes entre fichiers sources et headers
-include $(BUILD_DIR)/*.d
-include $(BUILD_DIR_DEBUG)/*.d

# ========== VERSIONS PROGRESSIVES (pour tests ELO) ==========

# Modules communs pour toutes les versions
COMMON_OBJ_V = $(BUILD_DIR)/board.o $(BUILD_DIR)/movegen.o $(BUILD_DIR)/utils.o \
               $(BUILD_DIR)/evaluation.o $(BUILD_DIR)/zobrist.o $(BUILD_DIR)/transposition.o \
               $(BUILD_DIR)/move_ordering.o $(BUILD_DIR)/quiescence.o $(BUILD_DIR)/search_helpers.o \
               $(BUILD_DIR)/uci.o $(BUILD_DIR)/timemanager.o $(BUILD_DIR)/main.o

# Création des dossiers versions si nécessaires
versions/v%_build:
	mkdir -p $@

# V1: Iterative Deepening + Alpha-Beta + Quiescence (baseline)
v1: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v1_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=1 -o versions/v1_build/chess_engine_v1 $^ -lm

# V2: + Move Ordering (MVV-LVA)
v2: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v2_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=2 -o versions/v2_build/chess_engine_v2 $^ -lm

# V3: + Transposition Table
v3: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v3_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=3 -o versions/v3_build/chess_engine_v3 $^ -lm

# V4: + Principal Variation Search (PVS)
v4: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v4_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=4 -o versions/v4_build/chess_engine_v4 $^ -lm

# V5: + Reverse Futility Pruning
v5: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v5_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=5 -o versions/v5_build/chess_engine_v5 $^ -lm

# V6: + Null Move Pruning
v6: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v6_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=6 -o versions/v6_build/chess_engine_v6 $^ -lm

# V7: + Late Move Reductions (LMR)
v7: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v7_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=7 -o versions/v7_build/chess_engine_v7 $^ -lm

# V8: + Butterfly History Heuristic
v8: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v8_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=8 -o versions/v8_build/chess_engine_v8 $^ -lm

# V9: + Killer Moves
v9: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v9_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=9 -o versions/v9_build/chess_engine_v9 $^ -lm

# V10: + Futility Pruning (VERSION COMPLÈTE)
v10: $(BUILD_DIR)/search.o $(COMMON_OBJ_V) | versions/v10_build
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=10 -o versions/v10_build/chess_engine_v10 $^ -lm

# Compilation de toutes les versions
all_versions: v1 v2 v3 v4 v5 v6 v7 v8 v9 v10

# Déclaration des cibles "virtuelles" pour éviter des conflits avec des fichiers du même nom
.PHONY: all debug release clean v1 v2 v3 v4 v5 v6 v7 v8 v9 v10 all_versions

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

# ========== CIBLES DE NETTOYAGE ==========

# Nettoyage basique : supprime les exécutables et dossiers build
clean:
	@echo "🧹 Nettoyage des builds principaux..."
	@rm -f chess_engine_debug
	@rm -rf $(BUILD_DIR) $(BUILD_DIR_DEBUG)
	@rm -rf chess_engine
	@echo "✅ Nettoyage terminé"

# Nettoyage des versions : supprime tous les dossiers versions/v*_build
clean-versions:
	@echo "🧹 Nettoyage des versions..."
	@rm -rf versions/v*_build
	@echo "✅ Versions nettoyées"

# Nettoyage des logs et fichiers temporaires
clean-logs:
	@echo "🧹 Nettoyage des logs..."
	@rm -rf logs/*.log logs/*.txt
	@rm -rf pgn_results/*.pgn
	@echo "✅ Logs nettoyés"

# Nettoyage complet : tout supprimer (builds + versions + logs)
clean-all: clean clean-versions clean-logs
	@echo "🧹 Nettoyage complet..."
	@rm -f *.o *.d *.dSYM
	@rm -rf *.dSYM
	@echo "✅ Nettoyage complet terminé"

# Alias pour clean-all
distclean: clean-all

# Inclusion des fichiers de dépendances automatiques générés lors de la compilation
# Cela permet à make de connaître les dépendances exactes entre fichiers sources et headers
-include $(BUILD_DIR)/*.d
-include $(BUILD_DIR_DEBUG)/*.d

# ========== VERSIONS PROGRESSIVES (pour tests ELO) ==========

# Modules sources communs
MODULES_SRC = Engine/board.c Engine/movegen.c Engine/utils.c Engine/evaluation.c \
              Engine/zobrist.c Engine/transposition.c Engine/move_ordering.c \
              Engine/quiescence.c Engine/search_helpers.c Engine/perft.c \
              Engine/uci.c Engine/timemanager.c Engine/search.c Engine/main.c

# Création des dossiers versions si nécessaires
versions/v%_build:
	mkdir -p $@

# Fonction pour compiler une version avec son propre VERSION
define COMPILE_VERSION
v$(1): | versions/v$(1)_build
	@echo "🔨 Compilation de la version $(1)..."
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -DVERSION=$(1) $(MODULES_SRC) -o versions/v$(1)_build/chess_engine_v$(1) -lm
	@echo "✅ Version $(1) compilée"
endef

# Générer les règles pour chaque version
$(foreach v,1 2 3 4 5 6 7 8 9 10 11 12 13,$(eval $(call COMPILE_VERSION,$(v))))

# Création du dossier chess_engine pour la version actuelle
chess_engine_dir:
	@rm -rf chess_engine
	@mkdir -p chess_engine

# Version actuelle (complète) dans le dossier chess_engine
current: $(OBJ_RELEASE) | chess_engine_dir
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o chess_engine/chess_engine $^ -lm

# Compilation de toutes les versions
all_versions: v1 v2 v3 v4 v5 v6 v7 v8 v9 v10 v11 v12 v13

# Rebuild complet : clean + recompile everything
rebuild: clean-all all

# Rebuild des versions : clean versions + recompile all versions
rebuild-versions: clean-versions all_versions

# ========== CIBLES D'AIDE ==========

# Affiche l'aide sur les commandes disponibles
help:
	@echo "📖 Commandes Make disponibles :"
	@echo ""
	@echo "  🔨 COMPILATION :"
	@echo "    make              - Compile la version release (défaut)"
	@echo "    make release      - Compile la version release"
	@echo "    make debug        - Compile la version debug"
	@echo "    make current      - Compile la version actuelle dans chess_engine/"
	@echo "    make all_versions - Compile toutes les versions (v1-v13)"
	@echo "    make v1..v13      - Compile une version spécifique"
	@echo ""
	@echo "  🧹 NETTOYAGE :"
	@echo "    make clean            - Nettoie builds + exécutables principaux"
	@echo "    make clean-versions   - Nettoie toutes les versions"
	@echo "    make clean-logs       - Nettoie les logs et PGN"
	@echo "    make clean-all        - Nettoyage complet (tout)"
	@echo "    make distclean        - Alias pour clean-all"
	@echo ""
	@echo "  🔄 REBUILD :"
	@echo "    make rebuild          - Clean + rebuild release"
	@echo "    make rebuild-versions - Clean versions + rebuild all"
	@echo ""
	@echo "  📚 AUTRES :"
	@echo "    make help             - Affiche cette aide"
	@echo ""

# Déclaration des cibles "virtuelles" pour éviter des conflits avec des fichiers du même nom
.PHONY: all debug release clean clean-versions clean-logs clean-all distclean \
        v1 v2 v3 v4 v5 v6 v7 v8 v9 v10 v11 v12 v13 all_versions current rebuild rebuild-versions help
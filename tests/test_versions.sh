#!/bin/bash



# Ce script lance un tournoi round-robin entre toutes les versions du moteur
# pour mesurer la progression de l'ELO.

# Assurer que le script s'arrête si une commande échoue
set -e

# Déterminer le chemin absolu de la racine du projet, peu importe d'où le script est lancé.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." > /dev/null && pwd)"

# --- Paramètres --- 
FASTCHESS_EXE="$PROJECT_ROOT/fastchess-mac-arm64/fastchess"
OPENING_FILE="$PROJECT_ROOT/openings/8moves_v3.pgn" # Fichier d'ouvertures PGN
GAMES=30
ROUNDS=1
TOTAL_VERSIONS=10

# Créer les chemins absolus pour les logs et pgn
LOG_DIR="$PROJECT_ROOT/logs/all_versions"
PGN_DIR="$PROJECT_ROOT/pgn_results/all_versions"

# --- Vérifications initiales ---

# Vérifier si fastchess est présent
if [ ! -f "$FASTCHESS_EXE" ]; then
    echo "ERREUR: Exécutable fastchess non trouvé à l'emplacement '$FASTCHESS_EXE'"
    echo "Veuillez le télécharger et le placer à la racine du projet."
    exit 1
fi

# Rendre l'exécutable
chmod +x "$FASTCHESS_EXE"

# Créer les dossiers de résultats s'ils n'existent pas
mkdir -p "$PROJECT_ROOT/openings"
mkdir -p "$LOG_DIR"
mkdir -p "$PGN_DIR"

# Vérifier si le fichier d'ouvertures existe
if [ ! -f "$OPENING_FILE" ]; then
    echo "AVERTISSEMENT: Fichier d'ouvertures non trouvé à '$OPENING_FILE'"
    echo "Veuillez créer ou télécharger un fichier PGN d'ouvertures pour des tests variés."
    exit 1
fi

# --- Lancement du tournoi ---

echo "Lancement du tournoi Round-Robin pour les $TOTAL_VERSIONS versions..."

# Boucle pour le tournoi round-robin (chaque version affronte les versions supérieures)
for i in $(seq 1 $TOTAL_VERSIONS); do
  for j in $(seq $((i + 1)) $TOTAL_VERSIONS); do
    
    ENGINE1_NAME="v${i}"
    ENGINE2_NAME="v${j}"
    
    ENGINE1_PATH="$PROJECT_ROOT/versions/${ENGINE1_NAME}_build/chess_engine_${ENGINE1_NAME}"
    ENGINE2_PATH="$PROJECT_ROOT/versions/${ENGINE2_NAME}_build/chess_engine_${ENGINE2_NAME}"

    LOG_FILE="$LOG_DIR/${ENGINE1_NAME}_vs_${ENGINE2_NAME}.log"
    PGN_FILE="$PGN_DIR/${ENGINE1_NAME}_vs_${ENGINE2_NAME}.pgn"

    echo ""
    echo "----------------------------------------------------"
    echo "Match en cours : $ENGINE1_NAME vs $ENGINE2_NAME"
    echo "----------------------------------------------------"

    # Lancement de fastchess pour le match
    "$FASTCHESS_EXE" -quick \
      -engine cmd="$ENGINE1_PATH" name="$ENGINE1_NAME" \
      -engine cmd="$ENGINE2_PATH" name="$ENGINE2_NAME" \
      -games "$GAMES" \
      -rounds "$ROUNDS" \
      -openings file="$OPENING_FILE" format=pgn order=random \
      -log file="$LOG_FILE" engine=true \
      -pgnout file="$PGN_FILE" notation=uci

    echo "Match terminé. PGN sauvegardé dans $PGN_FILE"
  done
done

echo ""
echo "----------------------------------------------------"
echo "Tournoi terminé !"
echo "Tous les résultats PGN se trouvent dans le dossier pgn_results/all_versions/"
echo "----------------------------------------------------"

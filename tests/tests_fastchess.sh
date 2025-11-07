#!/bin/bash

# === Configuration ===
FASTCHESS="./fastchess-mac-arm64/fastchess"  # chemin vers fastchess (adapter si besoin)
OPENINGS="./fastchess-mac-arm64/openings/8moves_v3.pgn"  # chemin vers le fichier d'ouvertures

# === Étape 1 : Saisie utilisateur ===
read -p "Chemin du moteur 1 : " ENGINE1
read -p "Chemin du moteur 2 : " ENGINE2
read -p "Nombre de parties (games) : " GAMES

# === Étape 2 : Détermination du numéro de fichier ===
LOG_DIR="./fastchess-mac-arm64/logs"
PGN_DIR="./fastchess-mac-arm64/pgn_results"

mkdir -p "$LOG_DIR" "$PGN_DIR"

# Trouver le prochain numéro disponible
n=1
while [ -e "$LOG_DIR/parties$n.log" ] || [ -e "$PGN_DIR/resultats$n.pgn" ]; do
  n=$((n+1))
done

LOG_FILE="$LOG_DIR/parties$n.log"
PGN_FILE="$PGN_DIR/resultats$n.pgn"

# === Étape 3 : Lancement du match ===
echo "⚔️  Lancement du match entre :"
echo "  - $ENGINE1"
echo "  - $ENGINE2"
echo "  → Résultats : $LOG_FILE et $PGN_FILE"
echo

$FASTCHESS -quick \
  cmd="$ENGINE1" \
  cmd="$ENGINE2" \
  -games "$GAMES" \
  -rounds 1 \
  -openings file="$OPENINGS" format=pgn order=random \
  -log file="$LOG_FILE" engine=true \
  -pgnout file="$PGN_FILE" notation=uci

# === Étape 4 : Découpage des logs ===
SPLIT_DIR="$LOG_DIR/parties${n}_split"
mkdir -p "$SPLIT_DIR"

echo "✂️  Découpage du log partie par partie..."
csplit -z -f "$SPLIT_DIR/game_" "$LOG_FILE" '/^Started game/' '{*}' >/dev/null 2>&1

echo "✅ Terminé."
echo "→ Logs individuels disponibles dans : $SPLIT_DIR"
echo "→ Résumé global : $LOG_FILE"
echo "→ PGN : $PGN_FILE"
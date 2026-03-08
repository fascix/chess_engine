#!/bin/bash

# === Configuration ===
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FASTCHESS="$SCRIPT_DIR/../fastchess-mac-arm64/fastchess"  # chemin vers fastchess (adapter si besoin)
OPENINGS="$SCRIPT_DIR/../fastchess-mac-arm64/openings/8moves_v3.pgn"  # chemin vers le fichier d'ouvertures

# === Étape 1 : Saisie utilisateur ===
read -p "Chemin du moteur 1 : " ENGINE1
read -p "Chemin du moteur 2 : " ENGINE2
read -p "Nombre de parties (games) : " GAMES

# === Étape 2 : Détermination du numéro de fichier ===
LOG_DIR="$SCRIPT_DIR/../fastchess-mac-arm64/logs"
PGN_DIR="$SCRIPT_DIR/../fastchess-mac-arm64/pgn_results"

mkdir -p "$LOG_DIR" "$PGN_DIR"

# Trouver le prochain numéro disponible
n=1
while [ -e "$LOG_DIR/parties$n.log" ] || [ -e "$PGN_DIR/resultats$n.pgn" ]; do
  n=$((n+1))
done

LOG_FILE="$LOG_DIR/parties$n.log"
PGN_FILE="$PGN_DIR/resultats$n.pgn"

# === Étape 3 : Lancement du match ===
if [ ! -f "$FASTCHESS" ]; then
  echo "❌ Erreur : Le binaire fastchess n'a pas été trouvé à l'emplacement : $FASTCHESS"
  exit 1
fi

if [ ! -f "$OPENINGS" ]; then
  echo "❌ Erreur : Le fichier d'ouvertures n'a pas été trouvé à l'emplacement : $OPENINGS"
  exit 1
fi

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

if [ $? -ne 0 ]; then
  echo "❌ Erreur lors de l'exécution de fastchess."
  exit 1
fi

# === Étape 4 : Découpage des logs ===
if [ -s "$LOG_FILE" ]; then
  SPLIT_DIR="$LOG_DIR/parties${n}_split"
  mkdir -p "$SPLIT_DIR"

  echo "✂️  Découpage du log partie par partie..."
  csplit -z -f "$SPLIT_DIR/game_" "$LOG_FILE" '/^Started game/' '{*}' >/dev/null 2>&1
else
  echo "⚠️  Attention : Le fichier de log est vide ou n'existe pas, pas de découpage possible."
fi

echo "✅ Terminé."
echo "→ Logs individuels disponibles dans : $SPLIT_DIR"
echo "→ Résumé global : $LOG_FILE"
echo "→ PGN : $PGN_FILE"
#!/bin/bash

# =============================================================================
# 🏛️  PALLAS CHESS ENGINE - Script d'installation
# =============================================================================

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}🏛️  Installation de Pallas Chess Engine & GUI...${NC}"
echo "--------------------------------------------------"

# 1. Vérification des dépendances système
echo -e "\n${YELLOW}[1/4] Vérification des dépendances système...${NC}"

# Vérifier GCC/Clang
if command -v gcc >/dev/null 2>&1; then
    echo -e "${GREEN}✓ GCC est installé.${NC}"
elif command -v clang >/dev/null 2>&1; then
    echo -e "${GREEN}✓ Clang est installé.${NC}"
else
    echo -e "${RED}✗ Aucun compilateur C (gcc/clang) trouvé. Veuillez l'installer.${NC}"
    exit 1
fi

# Vérifier Make
if command -v make >/dev/null 2>&1; then
    echo -e "${GREEN}✓ Make est installé.${NC}"
else
    echo -e "${RED}✗ Make n'est pas installé. Veuillez l'installer.${NC}"
    exit 1
fi

# Vérifier Python3
if command -v python3 >/dev/null 2>&1; then
    PYTHON_CMD="python3"
    echo -e "${GREEN}✓ Python 3 est installé.${NC}"
elif command -v python >/dev/null 2>&1; then
    PYTHON_CMD="python"
    echo -e "${GREEN}✓ Python est installé.${NC}"
else
    echo -e "${RED}✗ Python n'est pas installé. Veuillez l'installer.${NC}"
    exit 1
fi

# Vérifier Pip
if command -v pip3 >/dev/null 2>&1; then
    PIP_CMD="pip3"
    echo -e "${GREEN}✓ Pip3 est installé.${NC}"
elif command -v pip >/dev/null 2>&1; then
    PIP_CMD="pip"
    echo -e "${GREEN}✓ Pip est installé.${NC}"
else
    echo -e "${RED}✗ Pip n'est pas installé. Veuillez l'installer.${NC}"
    exit 1
fi

# 2. Installation des dépendances Python
echo -e "\n${YELLOW}[2/4] Installation des dépendances Python...${NC}"
if [ -f "requirements.txt" ]; then
    $PIP_CMD install -r requirements.txt
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Dépendances Python installées avec succès.${NC}"
    else
        echo -e "${RED}✗ Erreur lors de l'installation des dépendances Python.${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ Fichier requirements.txt introuvable.${NC}"
fi

# 3. Compilation du moteur (Engine)
echo -e "\n${YELLOW}[3/4] Compilation du moteur Pallas...${NC}"
make clean
make pallas
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Moteur compilé avec succès (exécutable: ./pallas).${NC}"
else
    echo -e "${RED}✗ Erreur lors de la compilation du moteur.${NC}"
    exit 1
fi

# 4. Finalisation
echo -e "\n${YELLOW}[4/4] Configuration finale...${NC}"
# Créer les dossiers nécessaires s'ils n'existent pas
mkdir -p logs
mkdir -p GUI/logs

echo -e "\n${GREEN}✨ Installation terminée avec succès !${NC}"
echo "--------------------------------------------------"
echo -e "Pour lancer l'interface graphique :"
echo -e "${BLUE}  cd GUI && $PYTHON_CMD main.py${NC}"
echo ""
echo -e "Pour utiliser le moteur en ligne de commande :"
echo -e "${BLUE}  ./pallas${NC}"
echo "--------------------------------------------------"

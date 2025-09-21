#!/usr/bin/env python3
# Sauvegardez ce fichier comme python_gui/test_engine.py
"""
Script de test pour vérifier le moteur UCI indépendamment du GUI
"""
import chess
import chess.engine
import os
import sys

def test_engine_paths():
    """Teste différents chemins pour trouver le moteur."""
    possible_paths = [
        "./chess_uci",          # Si lancé depuis la racine
        "../chess_uci",         # Si lancé depuis python_gui
        "chess_uci",            # Dans le PATH
        "./src/chess_uci"       # Si le binaire est dans src/
    ]
    
    print(f"Working directory: {os.getcwd()}")
    print("Recherche du moteur UCI...")
    
    for path in possible_paths:
        abs_path = os.path.abspath(path)
        exists = os.path.exists(path)
        executable = os.access(path, os.X_OK) if exists else False
        
        print(f"  {path} -> {abs_path}")
        print(f"    Existe: {exists}, Exécutable: {executable}")
        
        if exists and executable:
            print(f"✅ Moteur trouvé: {path}")
            return path
    
    print("❌ Aucun moteur trouvé!")
    return None

def test_engine_communication(engine_path):
    """Teste la communication avec le moteur UCI."""
    try:
        print(f"\n🔧 Test de communication avec {engine_path}")
        
        # Créer un échiquier de test
        board = chess.Board()
        print(f"Position initiale: {board.fen()}")
        
        # Démarrer le moteur
        with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
            print("✅ Moteur démarré avec succès")
            
            # Test 1: Premier coup des blancs
            print("\n🎯 Test 1: Premier coup des blancs")
            result1 = engine.play(board, chess.engine.Limit(time=0.1))
            print(f"Coup suggéré: {result1.move}")
            
            # Appliquer le coup
            board.push(result1.move)
            print(f"Position après le coup: {board.fen()}")
            
            # Test 2: Réponse des noirs
            print("\n🎯 Test 2: Réponse des noirs")
            result2 = engine.play(board, chess.engine.Limit(time=0.1))
            print(f"Coup suggéré: {result2.move}")
            
            # Test 3: Position spécifique
            print("\n🎯 Test 3: Position après 1.e4")
            test_board = chess.Board()
            test_board.push_san("e4")  # 1.e4
            result3 = engine.play(test_board, chess.engine.Limit(time=0.1))
            print(f"Position: {test_board.fen()}")
            print(f"Réponse des noirs: {result3.move}")
            
        print("\n✅ Tous les tests de communication réussis!")
        return True
        
    except FileNotFoundError:
        print(f"❌ Erreur: Fichier {engine_path} non trouvé")
        return False
    except Exception as e:
        print(f"❌ Erreur de communication: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    print("=== Test du moteur UCI ===\n")
    
    # Étape 1: Trouver le moteur
    engine_path = test_engine_paths()
    if not engine_path:
        print("\n❌ Impossible de continuer sans moteur")
        sys.exit(1)
    
    # Étape 2: Tester la communication
    if test_engine_communication(engine_path):
        print("\n🎉 Le moteur fonctionne correctement!")
        print(f"Utilisez ce chemin dans votre code: '{engine_path}'")
    else:
        print("\n❌ Problème avec le moteur")
        sys.exit(1)

if __name__ == "__main__":
    main()
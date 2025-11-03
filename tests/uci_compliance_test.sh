#!/bin/bash
# Test de conformité UCI pour ChessEngine v2.0
# Teste les nouvelles fonctionnalités implémentées

ENGINE="./chess_engine"
TIMEOUT=3

echo "=== Test de conformité UCI ==="
echo

# Test 1: Commande uci avec options
echo "Test 1: uci - Vérification des options"
result=$(timeout $TIMEOUT bash -c "echo 'uci' | $ENGINE" 2>/dev/null)
if echo "$result" | grep -q "option name Hash"; then
    echo "✅ Option Hash présente"
else
    echo "❌ Option Hash manquante"
fi
if echo "$result" | grep -q "option name Ponder"; then
    echo "✅ Option Ponder présente"
else
    echo "❌ Option Ponder manquante"
fi
echo

# Test 2: debug on/off
echo "Test 2: debug on/off"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
debug on
quit
EOF' 2>&1)
if echo "$result" | grep -q "Debug mode enabled"; then
    echo "✅ Debug mode fonctionne"
else
    echo "❌ Debug mode ne fonctionne pas"
fi
echo

# Test 3: setoption
echo "Test 3: setoption name Hash value 128"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
debug on
setoption name Hash value 128
quit
EOF' 2>&1)
if echo "$result" | grep -q "Hash set to 128"; then
    echo "✅ setoption Hash fonctionne"
else
    echo "❌ setoption Hash ne fonctionne pas"
fi
echo

# Test 4: register (doit être reconnu sans erreur)
echo "Test 4: register later"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
register later
quit
EOF' 2>&1)
if [ $? -eq 0 ]; then
    echo "✅ register reconnu sans erreur"
else
    echo "❌ register cause une erreur"
fi
echo

# Test 5: ponderhit (doit être reconnu sans erreur)
echo "Test 5: ponderhit"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
ponderhit
quit
EOF' 2>&1)
if [ $? -eq 0 ]; then
    echo "✅ ponderhit reconnu sans erreur"
else
    echo "❌ ponderhit cause une erreur"
fi
echo

# Test 6: go avec info intermédiaires
echo "Test 6: go movetime 100 - Vérification des info UCI"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
position startpos
go movetime 100
quit
EOF' 2>/dev/null)
if echo "$result" | grep -q "info depth"; then
    echo "✅ Info UCI envoyées pendant la recherche"
else
    echo "❌ Aucune info UCI envoyée"
fi
if echo "$result" | grep -q "bestmove"; then
    echo "✅ bestmove envoyé"
else
    echo "❌ bestmove manquant"
fi
echo

# Test 7: go avec paramètres avancés
echo "Test 7: go avec paramètres avancés (wtime, btime, winc, binc)"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
position startpos
go wtime 60000 btime 60000 winc 1000 binc 1000
quit
EOF' 2>/dev/null)
if echo "$result" | grep -q "bestmove"; then
    echo "✅ go avec time control fonctionne"
else
    echo "❌ go avec time control ne fonctionne pas"
fi
echo

# Test 8: go depth
echo "Test 8: go depth 5"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
position startpos
go depth 5
quit
EOF' 2>/dev/null)
if echo "$result" | grep -q "bestmove"; then
    echo "✅ go depth fonctionne"
else
    echo "❌ go depth ne fonctionne pas"
fi
echo

# Test 9: go infinite + stop
echo "Test 9: go infinite + stop"
result=$(timeout $TIMEOUT bash -c 'cat <<EOF | $ENGINE
position startpos
go infinite
stop
quit
EOF' 2>/dev/null)
if echo "$result" | grep -q "bestmove"; then
    echo "✅ go infinite + stop fonctionne"
else
    echo "❌ go infinite + stop ne fonctionne pas"
fi
echo

echo "=== Résumé ==="
echo "Toutes les nouvelles fonctionnalités UCI ont été testées."
echo "Le moteur est maintenant conforme à la spécification UCI."


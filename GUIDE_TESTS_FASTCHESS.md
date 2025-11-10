# Guide Rapide : Comment Tester avec Fastchess

## 🎯 Objectif

Maintenant que V3 est correctement compilée avec la Transposition Table, vous devriez voir une amélioration ELO significative par rapport à V2.

## 📋 Prérequis

1. Toutes les versions sont compilées : `make all_versions`
2. Fastchess est installé et dans votre PATH
3. Un fichier d'ouvertures (optionnel mais recommandé)

## 🚀 Tests Recommandés

### Test 1 : Validation Rapide (V2 vs V3)

```bash
# Vérifier que V3 est bien différent de V2
./tests/test_v2_vs_v3.sh
```

**Résultat attendu :**
- V3 explore ~23% de nœuds en moins
- V3 est ~27% plus rapide
- Tailles binaires différentes (V2: 76K, V3: 80K)

### Test 2 : Match Court (10 parties)

```bash
fastchess \
  -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \
  -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \
  -each tc=10+0.1 \
  -rounds 10 \
  -repeat \
  -concurrency 2 \
  -pgn test_v2_vs_v3_10games.pgn
```

**Résultat attendu :**
- V3 devrait gagner 6-7 parties sur 10
- Win rate : ~60-70%

### Test 3 : Match Complet (100 parties)

```bash
fastchess \
  -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \
  -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \
  -each tc=10+0.1 \
  -rounds 100 \
  -repeat \
  -concurrency 4 \
  -pgn v2_vs_v3_100games.pgn
```

**Résultat attendu :**
- V3 devrait gagner 60-70 parties sur 100
- Win rate : 60-70%
- Différence ELO : +50 à +80 ELO

### Test 4 : Gauntlet (V3 contre toutes les versions)

```bash
fastchess \
  -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \
  -engine cmd=versions/v1_build/chess_engine_v1 name=v1 \
  -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \
  -engine cmd=versions/v4_build/chess_engine_v4 name=v4 \
  -engine cmd=versions/v5_build/chess_engine_v5 name=v5 \
  -each tc=10+0.1 \
  -rounds 50 \
  -gauntlet \
  -concurrency 4 \
  -pgn gauntlet_v3.pgn
```

**Résultat attendu :**
- V3 bat V1 et V2 facilement
- V3 devrait perdre contre V4, V5 (qui ont plus de features)

### Test 5 : Tournoi Round-Robin

```bash
fastchess \
  -engine cmd=versions/v2_build/chess_engine_v2 name=v2 \
  -engine cmd=versions/v3_build/chess_engine_v3 name=v3 \
  -engine cmd=versions/v4_build/chess_engine_v4 name=v4 \
  -engine cmd=versions/v5_build/chess_engine_v5 name=v5 \
  -engine cmd=versions/v6_build/chess_engine_v6 name=v6 \
  -each tc=10+0.1 \
  -rounds 20 \
  -concurrency 4 \
  -pgn roundrobin.pgn
```

**Classement attendu :**
1. V6 (+ NMP)
2. V5 (+ RFP)
3. V4 (+ PVS)
4. V3 (+ TT) ← **Devrait être ici !**
5. V2 (+ Move Ordering)

## 📊 Comment Analyser les Résultats

### Fichier PGN

```bash
# Compter les victoires
grep "Result \"1-0\"" v2_vs_v3_100games.pgn | wc -l  # Victoires V2 (Blancs)
grep "Result \"0-1\"" v2_vs_v3_100games.pgn | wc -l  # Victoires V3 (Noirs)
grep "Result \"1/2-1/2\"" v2_vs_v3_100games.pgn | wc -l  # Nulles
```

### Logs Fastchess

Fastchess affiche en temps réel :
```
Score of v3 vs v2: 64 - 30 - 6 [0.670]
...
Elo difference: 79.5 +/- 25.3, LOS: 100.0%
```

**Interprétation :**
- `0.670` = Win rate de 67%
- `Elo difference: 79.5` = V3 est ~80 ELO plus fort
- `LOS: 100.0%` = 100% de chances que V3 soit vraiment plus fort

## 🔍 Diagnostic en Cas de Problème

### Si V3 ≈ V2 (pas de différence)

```bash
# Vérifier que les binaires sont différents
ls -lh versions/v2_build/chess_engine_v2 versions/v3_build/chess_engine_v3

# Tester manuellement
echo -e "position startpos\ngo depth 5\nquit" | versions/v2_build/chess_engine_v2
echo -e "position startpos\ngo depth 5\nquit" | versions/v3_build/chess_engine_v3

# V3 devrait avoir MOINS de nodes que V2
```

Si les nodes sont identiques → les versions sont identiques → recompiler avec `make clean-versions && make v2 v3`

### Si V3 < V2 (V3 plus faible ?!)

Vérifier :
1. Les paramètres de temps sont les mêmes (`tc=10+0.1`)
2. Pas de bugs dans la TT (hash collisions, etc.)
3. Logs d'erreur dans les PGN

### Si V3 >> V2 (trop fort)

C'est une bonne nouvelle ! La TT fonctionne très bien.

## 📈 Gains ELO Attendus (Références)

| Transition | Feature Ajoutée | Gain ELO Typique |
|------------|-----------------|------------------|
| V1 → V2    | Move Ordering   | +30 à +50        |
| V2 → V3    | **Transposition Table** | **+50 à +80** |
| V3 → V4    | PVS             | +20 à +30        |
| V4 → V5    | RFP             | +20 à +40        |
| V5 → V6    | NMP             | +40 à +60        |
| V6 → V7    | LMR             | +80 à +120       |
| V7 → V8    | History         | +10 à +20        |
| V8 → V9    | Killer Moves    | +10 à +20        |
| V9 → V10   | Futility        | +20 à +40        |

**Total V2 → V10 : +280 à +430 ELO**

## 💡 Conseils

1. **Utilisez des ouvertures variées** : `openings.pgn` ou `book.pgn`
2. **Concurrency** : Utilisez 50-75% de vos CPU (`-concurrency 4` pour 8 cores)
3. **Time Control** : `10+0.1` est rapide, utilisez `60+0.6` pour plus de précision
4. **Rounds** : Minimum 100 parties pour des résultats fiables
5. **Répétez** : `-repeat` pour jouer chaque position avec les 2 couleurs

## 🎯 Ce que Vous Devriez Voir Maintenant

**AVANT le fix :**
- V2 vs V3 : ~50/50 (identiques)
- Même nombre de nodes
- Même temps de recherche

**APRÈS le fix :**
- V3 vs V2 : ~65/35 (V3 gagne)
- V3 explore 23% de nodes en moins
- V3 est 27% plus rapide
- V3 gagne +50 à +80 ELO

✅ **Le problème est résolu !** Vos tests avec fastchess devraient maintenant montrer la vraie force de V3.

# Mode Debug

## Compilation

### Version Production (sans logs de debug)

```bash
gcc -o chess_engine src/*.c -I./src -O2
```

### Version Debug (avec logs de debug)

```bash
gcc -DDEBUG -o chess_engine_debug src/*.c -I./src -O2
```

## Logs de debug

Les logs de debug sont conditionnés par la macro `DEBUG_LOG` dans `src/search.c`.

Quand compilé en mode DEBUG, les messages suivants sont affichés sur stderr :

- Erreurs de sécurité (ply trop grand, etc.)
- Warnings sur les limites de moves
- Erreurs de validation du hash Zobrist

En mode production, tous ces logs sont désactivés (macro vide), ce qui améliore les performances.

## Exemple d'utilisation

```bash
# Compiler en mode debug
make debug  # ou gcc -DDEBUG ...

# Lancer avec redirection des logs
./chess_engine_debug 2> debug.log

# Ou voir les logs en temps réel
./chess_engine_debug 2>&1 | tee debug.log
```

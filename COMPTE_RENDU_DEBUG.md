# Compte Rendu : Débogage des Coups Illégaux

Ce document résume les étapes suivies pour identifier et tenter de résoudre le problème des coups illégaux générés par le moteur d'échecs lors des tests avec `fastchess`.

## 1. Problème Initial

Le moteur générait des coups illégaux, principalement de deux types :

1.  **Coups sur la même case :** `a1a1`.
2.  **Coups invalides dans des positions complexes :** `e1e6`.

Ces erreurs provoquaient l'arrêt des parties et une défaite immédiate.

## 2. Itérations de Débogage et Correctifs

### Itération 1 : Initialisation des `Move` et Validation UCI

- **Hypothèse :** Les structures `Move` étaient initialisées avec `{0}`, ce qui correspond à un coup de `A1` vers `A1`. Si la recherche échouait à cause du temps, ce coup invalide pouvait être retourné.
- **Correctif Appliqué :**
  1.  Modification de l'initialisation des `Move` pour utiliser des marqueurs invalides explicites (`from = -1`, `to = -1`).
  2.  Ajout d'une validation stricte dans `handle_go` (fichier `uci.c`) pour vérifier que le coup retourné par la recherche est bien légal et n'est pas un coup sur la même case.
- **Résultat :** Le problème des coups `a1a1` a disparu, mais de nouveaux coups illégaux sont apparus (`f7e5`), indiquant un problème plus profond.

### Itération 2 : Désynchronisation de l'État "En Passant"

- **Hypothèse :** L'état de l'échiquier interne du moteur se désynchronisait de celui de `fastchess`. L'analyse a révélé un bug dans la fonction `make_move_temp`.
- **Analyse :** La condition pour définir la case "en passant" ne fonctionnait que pour les pions blancs (`move->to - move->from == 16`) et échouait pour les pions noirs (différence de `-16`).
- **Correctif Appliqué :** Remplacement de la condition par `abs(move->to - move->from) == 16` pour gérer les deux couleurs.
- **Résultat :** Le bug a persisté, révélant d'autres problèmes de désynchronisation, notamment liés aux droits de roque.

### Itération 3 : Gestion des Droits de Roque

- **Hypothèse :** De nouveaux coups illégaux liés au roque (`e8g8`) sont apparus. Cela suggérait que les droits de roque n'étaient pas correctement mis à jour.
- **Analyse :** Le code ne retirait pas le droit de roque lorsqu'une tour était **capturée** sur sa case de départ (ex: capture de la tour en `h8`). Le moteur pensait donc qu'il pouvait toujours roquer.
- **Correctif Appliqué :** Ajout d'une logique dans `make_move_temp` pour annuler le droit de roque correspondant si une tour est capturée sur l'une des cases `A1`, `H1`, `A8`, ou `H8`.
- **Résultat :** Encore un échec. Le problème était encore plus subtil, lié à la manière dont `fastchess` initialise les positions.

### Itération 4 : Logique de Correspondance des Coups UCI

- **Hypothèse :** Le moteur se désynchronisait lors de l'application de longues séquences de coups fournies par `fastchess`.
- **Analyse :** La fonction `apply_uci_moves` avait une logique trop simpliste. Elle ne différenciait pas un coup normal d'une capture ou d'un roque, se contentant de faire correspondre les cases de départ et d'arrivée. Dans des positions complexes, cela pouvait entraîner l'application d'un mauvais type de coup.
- **Correctif Appliqué :** Amélioration de la logique dans `apply_uci_moves` pour qu'elle trouve le coup légal correspondant dans la liste générée, en tenant compte des promotions.
- **Résultat :** Échec final. Le test a révélé des coups "hallucinés" (`c4e2` depuis une case vide), prouvant une corruption majeure de l'état de l'échiquier.

### Itération 5 : Le Bug Final - Corruption de la Pile de Recherche

- **Hypothèse :** La cause racine n'était pas un bug isolé, mais une interaction destructrice entre différentes parties de la recherche.
- **Analyse :** La recherche principale (`negamax`) et la recherche de quiétude (`quiescence_search`) utilisaient toutes les deux la même pile de sauvegarde (`search_backup_stack`) sans coordination. La recherche de quiétude écrasait les sauvegardes de la recherche principale, ce qui corrompait l'état de l'échiquier lors de la restauration (`undo_move`).
- **Correctif Appliqué :** Tentative de standardisation de la gestion de la pile de sauvegarde en passant la profondeur de recherche (`ply`) à la recherche de quiétude.
- **Résultat :** Échec. Mes tentatives de correction ont introduit des erreurs de compilation ou n'ont pas résolu le problème de fond.

## 3. Conclusion

Le problème fondamental est une **corruption de l'état de l'échiquier** qui se produit pendant la recherche à cause d'une mauvaise gestion de la pile de sauvegarde entre la recherche principale et la recherche de quiétude. Les correctifs appliqués ont résolu des bugs de surface mais ont échoué à corriger cette cause racine.

À ce stade, le problème dépasse mes capacités de débogage automatisé. Une intervention manuelle avec un outil comme **GDB** est nécessaire pour inspecter l'état de la variable `board` pas à pas et identifier précisément où et comment la corruption se produit.

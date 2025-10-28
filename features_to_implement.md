# Feuille de Route d'Implémentation : 15 Techniques Avancées pour le Moteur d'Échecs

Ce document détaille les 15 prochaines techniques à implémenter pour améliorer significativement la force du moteur, avec un gain ELO total estimé entre **+435 et +750 ELO**.

---

# 🔥 **1. Null Move Pruning (NMP)**

## 🧠 **Concept**

**Idée** : Si même en donnant un coup gratuit à l'adversaire (coup null = passer son tour), ma position reste si bonne qu'elle cause un cutoff (score ≥ beta), alors je peux couper cette branche car l'adversaire trouvera forcément encore mieux avec un vrai coup.

**Intuition** : Si je suis tellement bien positionné que même sans jouer je surpasse beta, inutile de chercher plus loin dans cette branche.

## 🔧 **Comment ça marche**

1.  **Conditions d'activation (CRITIQUES)**

    - `depth >= 3` → Profondeur suffisante pour éviter les erreurs tactiques.
    - `!is_in_check()` → Interdit en échec, car passer son tour est illégal.
    - `has_non_pawn_material()` → Désactivé en finale de pions pour éviter les problèmes de zugzwang.
    - `beta < MATE_SCORE` → Ne pas utiliser dans une séquence de mat.

2.  **Algorithme**

    ```c
    // Dans negamax, avant la génération des coups
    if (conditions_valides) {
        // 1. Jouer un "coup null" (inverser le trait)
        board->to_move = opponent;

        // 2. Recherche réduite (R=réduction, typiquement 2 ou 3)
        int null_score = -negamax(board, depth - R - 1, -beta, -beta + 1, opponent, ply + 1);

        // 3. Annuler le coup null
        board->to_move = color;

        // 4. Si la recherche null échoue haut (null_score >= beta)
        if (null_score >= beta) {
            return beta;  // Cutoff ! On élague la branche.
        }
    }
    ```

## 📊 **Gains Attendus**

- **Réduction de l'arbre** : ~30-50% de nœuds en moins.
- **Gain ELO** : **+60 à +100 ELO**.
- **Profondeur effective** : +1 à +2 plies en plus dans le même temps.

---

# 🔥 **2. Late Move Reductions (LMR)**

## 🧠 **Concept**

**Idée** : Les coups tardifs dans l'ordre de tri (après les 3-4 premiers) ont moins de chances d'être bons. On les recherche donc avec une **profondeur réduite**. Si un coup surprend et améliore alpha, on le re-recherche à profondeur complète.

**Intuition** : Avec un bon move ordering, les meilleurs coups sont en premier. Les coups 15-30 sont probablement faibles, donc cherchons-les rapidement pour confirmer qu'ils sont mauvais.

## 🔧 **Comment ça marche**

1.  **Conditions d'activation**

    - `depth >= 3`
    - `i >= 4` (après les 4 premiers coups)
    - Le coup est "quiet" (pas une capture, promotion, ou échec).

2.  **Algorithme**

    ```c
    // Dans la boucle de coups
    if (i == 0) {
        // PV : recherche complète
    } else {
        int reduction = 0;
        if (conditions_LMR_valides) {
            // Formule de réduction (ex: log(depth) * log(i))
            reduction = calculer_reduction(depth, i);
        }

        // Recherche avec profondeur réduite
        score = -negamax(board, depth - 1 - reduction, -alpha - 1, -alpha, ...);

        // Re-search si le coup surprend
        if (score > alpha && reduction > 0) {
            score = -negamax(board, depth - 1, -alpha - 1, -alpha, ...);
        }

        // Re-search PVS si nécessaire
        if (score > alpha && score < beta) { ... }
    }
    ```

## 📊 **Gains Attendus**

- **Gain ELO** : **+80 à +120 ELO**.
- **Réduction de l'arbre** : ~40% de nœuds en moins.

---

# 🦋 **3. Butterfly History Heuristic**

## 🧠 **Concept**

**Idée** : Au lieu de scorer l'historique par `[color][from][to]`, on score par **`[from][to]`** indépendamment de la couleur. Cela capture les "patterns" de coups qui fonctionnent bien sur l'échiquier (ex: contrôle du centre).

**Intuition** : Certaines cases-clés (e4, d4, e5, d5) sont importantes quelle que soit la couleur. Le Butterfly History unifie les statistiques pour mieux détecter ces patterns.

## 🔧 **Comment ça marche**

1.  **Nouvelle structure** : Remplacer `history_scores[2][64][64]` par `butterfly_history[64][64]`.
2.  **Mise à jour** : Après un beta cutoff, incrémenter `butterfly_history[move.from][move.to] += depth * depth`.
3.  **Utilisation** : Dans `order_moves`, le score d'un coup quiet est directement `butterfly_history[move.from][move.to]`.

## 📊 **Gains Attendus**

- **Gain ELO** : **+10 à +20 ELO**.
- **Meilleur move ordering** pour les coups positionnels.

---

# ⚡ **4. Reverse Futility Pruning (RFP)**

## 🧠 **Concept**

**Idée** : Si ma position est **TROP bonne** (eval >> beta + marge), je peux couper directement sans chercher. L'adversaire ne pourra probablement pas nous rattraper.

**Intuition** : Si j'ai +6 pions d'avance et beta = +100, même avec ses meilleurs coups, l'adversaire ne peut pas empêcher un cutoff. Gagnons du temps.

## 🔧 **Comment ça marche**

1.  **Conditions**

    - `depth <= 7` (profondeur faible uniquement).
    - `!is_in_check()`.
    - `eval - margin >= beta`.

2.  **Algorithme**

    ```c
    // Avant la génération des coups
    if (conditions_RFP_valides) {
        int static_eval = evaluate_position(board);
        int margin = 120 * depth; // Marge par profondeur

        if (static_eval - margin >= beta) {
            return static_eval - margin; // Cutoff anticipé
        }
    }
    ```

## 📊 **Gains Attendus**

- **Gain ELO** : **+20 à +40 ELO**.
- **Réduction de l'arbre** : ~15% de nœuds en moins.

---

# 🗡️ **5. Futility Pruning**

## 🧠 **Concept**

**Idée** : Si ma position est **TROP mauvaise** (eval + marge_optimiste < alpha), je peux **ignorer les coups quiets restants** car même le meilleur coup ne peut pas améliorer alpha.

**Intuition** : Si j'ai -500 centipawns et alpha = +100, et qu'il ne me reste que des coups "normaux" (pas de captures), je ne peux pas remonter. Ignorons-les.

## 🔧 **Comment ça marche**

1.  **Conditions**

    - `depth <= 4` (profondeur très faible).
    - `!is_in_check()`.
    - Le coup est "quiet".
    - `eval + margin < alpha`.

2.  **Algorithme**

    ```c
    // Dans la boucle de coups, avant d'appliquer un coup quiet
    if (conditions_futility_valides) {
        int static_eval = evaluate_position(board);
        int margin = 200 * depth;

        if (static_eval + margin < alpha) {
            continue; // Ignore ce coup quiet
        }
    }
    ```

## 📊 **Gains Attendus**

- **Gain ELO** : **+30 à +50 ELO**.
- **Réduction de l'arbre** : ~20% de nœuds en moins.

---

# ✂️ **6. Late Move Pruning (LMP)**

## 🧠 **Concept**

**Idée** : Après avoir exploré un certain nombre de coups (ex: 15), si aucun n'a amélioré alpha, on peut **stopper complètement** l'exploration des coups quiets restants.

**Intuition** : Si les 15 premiers coups (bien triés) n'ont pas battu alpha, les coups 16-30 (mal triés) ne le feront probablement pas non plus.

## 🔧 **Comment ça marche**

- Définir un seuil de coups à essayer (`lmp_threshold`) basé sur la profondeur.
- Dans la boucle de coups, si `moves_tried >= lmp_threshold` et que le coup est quiet, `continue`.

## 📊 **Gains Attendus**

- **Gain ELO** : **+40 à +60 ELO**.
- **Réduction de l'arbre** : ~25% de nœuds en moins.

---

# 🔄 **7. Internal Iterative Reductions (IIR)**

## 🧠 **Concept**

**Idée** : Si on arrive dans un nœud **important** (PV node) mais qu'on n'a **PAS de hash move** en TT, c'est suspect. On fait une **recherche réduite** (ex: depth-2) d'abord pour obtenir un bon coup, puis on recherche normalement.

**Intuition** : Un PV node sans hash move signifie qu'on n'a jamais exploré cette position en profondeur. Plutôt que de chercher aveuglément, faisons une passe rapide pour trier les coups.

## 🔧 **Comment ça marche**

- Avant `order_moves`, si `!hash_move_valid && depth >= 6 && is_pv_node`, lancer une recherche à `depth - 2`.
- Récupérer le nouveau hash move de la TT et continuer normalement.

## 📊 **Gains Attendus**

- **Gain ELO** : **+15 à +30 ELO**.

---

# 📈 **8. Improving Heuristic**

## 🧠 **Concept**

**Idée** : Détecter si notre position **s'améliore** ou **se dégrade** au fil des coups. Si elle s'améliore, être plus **conservateur** dans les élagages. Si elle se dégrade, être plus **agressif**.

## 🔧 **Comment ça marche**

- Comparer l'évaluation statique actuelle avec celle de 2 plies auparavant (`eval_stack[ply - 2]`).
- Ajuster les paramètres de tous les élagages (NMP, LMR, Futility) en fonction du flag `improving`.

## 📊 **Gains Attendus**

- **Gain ELO** : **+20 à +40 ELO** (synergie avec les autres élagages).

---

# 🧬 **9. Continuation History (CMH + FMH)**

## 🧠 **Concept**

**Idée** : Scorer les **paires de coups** :

- **Counter-Move History (CMH)** : "Quel coup répond bien au coup adverse ?"
- **Follow-Up Move History (FMH)** : "Quel coup suit bien notre dernier coup ?"

**Intuition** : Les coups ne sont pas indépendants. Si l'adversaire joue Nf3, répondre par ...Nc6 est souvent bon.

## 🔧 **Comment ça marche**

- Utiliser des tables 4D pour stocker les scores des paires de coups.
- Mettre à jour après chaque beta cutoff.
- Dans `order_moves`, ajouter des bonus basés sur les scores CMH et FMH.

## 📊 **Gains Attendus**

- **Gain ELO** : **+30 à +50 ELO**.

---

# 🎯 **10. Capture History Heuristic**

## 🧠 **Concept**

**Idée** : Appliquer l'historique **uniquement aux captures**. Certaines captures (ex: BxN) fonctionnent souvent bien, d'autres (ex: QxP sur case piégée) échouent souvent.

**Intuition** : Le MVV-LVA seul est insuffisant. L'historique des captures permet d'apprendre ces patterns.

## 🔧 **Comment ça marche**

- Utiliser une table `capture_history[color][piece_captured][to_square]`.
- Mettre à jour en positif (beta cutoff) ou négatif (fail-low) après chaque capture explorée.
- Dans `order_moves`, ajuster le score MVV-LVA avec le score de l'historique des captures.

## 📊 **Gains Attendus**

- **Gain ELO** : **+20 à +40 ELO**.

---

# 🌳 **11. History Pruning**

## 🧠 **Concept**

**Idée** : Si un coup quiet a un score d'historique très faible (voire négatif), on peut l'élaguer agressivement, surtout à faible profondeur. C'est une forme de Futility Pruning basée sur des données historiques.

**Intuition** : Si un coup a constamment échoué à produire un cutoff par le passé, il est probablement inutile de le chercher maintenant, surtout si notre position n'est pas déjà excellente.

## 🔧 **Comment ça marche**

1.  **Conditions**

    - `depth <= 4`
    - Le coup est "quiet".
    - `butterfly_history[from][to] < 0` (un seuil négatif est une bonne base).

2.  **Algorithme**
    ```c
    // Dans la boucle de coups de negamax
    if (depth <= 4 && move_is_quiet(move)) {
        if (butterfly_history[move.from][move.to] < 0) {
            continue; // Élague ce coup
        }
    }
    ```

## 📊 **Gains Attendus**

- **Gain ELO** : **+20 à +30 ELO**.
- Élague des coups "silencieusement mauvais" que d'autres heuristiques pourraient manquer.

---

# singularity **12. Singular Extensions**

## 🧠 **Concept**

**Idée** : Une technique d'extension très puissante. Si un coup provenant de la table de transposition est **significativement meilleur** que tous les autres coups, il pourrait s'agir d'un coup "singulier" qui mérite une profondeur de recherche beaucoup plus grande.

**Intuition** : Si un coup mène à un mat forcé ou gagne une dame, tandis que tous les autres coups perdent, nous devons explorer ce coup gagnant très profondément pour confirmer sa force et ne pas le manquer.

## 🔧 **Comment ça marche**

1.  **Vérification de la singularité** (dans `negamax`)

    - On a un `tt_move` de la TT avec un score `tt_score`.
    - On fait une recherche rapide (ex: `depth - 4`) de **tous les autres coups** en excluant `tt_move`.
    - Cette recherche est faite avec une fenêtre serrée autour de `tt_score` pour trouver rapidement le `second_best_score`.

2.  **Condition d'extension**

    - `margin = tt_score - second_best_score`.
    - Si `margin > SINGULAR_MARGIN` (ex: `margin > 200`), le coup est singulier.

3.  **Algorithme**
    ```c
    // Après avoir trouvé un tt_move valide
    if (depth >= 8 && tt_move_is_valid && is_pv_node) {
        // 1. Recherche de vérification sans le tt_move
        int verification_depth = (depth / 2) - 1;
        int verification_beta = tt_score - 200; // Fenêtre serrée
        // Recherche en excluant tt_move
        int second_best_score = -negamax(..., verification_depth, verification_beta - 1, verification_beta, ...);

        // 2. Si le tt_move est bien meilleur
        if (second_best_score < verification_beta) {
            // 3. C'est un coup singulier ! On l'étend.
            int extension = 1;
            // Re-recherche du tt_move avec extension
            score = -negamax(board, depth - 1 + extension, -beta, -alpha, ...);
            return score;
        }
    }
    ```

## 📊 **Gains Attendus**

- **Gain ELO** : **+50 à +100 ELO**. C'est une fonctionnalité majeure.
- Améliore considérablement la résolution tactique.

---

# ⚔️ **13. Multicut**

## 🧠 **Concept**

**Idée** : Une généralisation avancée de l'élagage alpha-beta. Si le nombre de coups qui battent `alpha` dépasse un certain seuil, on peut parfois prouver que le nœud parent causera un beta cutoff sans avoir à chercher les frères restants.

**Intuition** : Si nous avons déjà trouvé 3 coups qui améliorent notre score (`> alpha`), la probabilité que le score final de ce nœud dépasse `beta` est très élevée. On peut faire une supposition statistique et élaguer.

## 🔧 **Comment ça marche**

```c
// Dans la boucle de coups
int fail_high_count = 0;
int multicut_threshold = 3 + depth / 2;

for (int i = 0; i < count; i++) {
    // ...
    score = -negamax(...);

    if (score > alpha) {
        fail_high_count++;
    }

    // Multicut Pruning
    if (fail_high_count >= multicut_threshold && i >= multicut_threshold && move_is_quiet(move)) {
         return beta; // Élague les coups quiets restants
    }
    // ...
}
```

## 📊 **Gains Attendus**

- **Gain ELO** : **+20 à +40 ELO**.
- Fonctionne bien en synergie avec d'autres techniques d'élagage.

---

# ➕➖ **14. Double/Triple/Negative Extensions**

## 🧠 **Concept**

**Idée** : Ce sont des ajustements fins de la logique des Singular Extensions pour moduler la quantité d'extension.

- **Double Extension (+2)** : Si un coup est singulier **ET** qu'il met le roi en échec, il est très probablement forçant. On l'étend de +2.
- **Negative Extension (-1)** : Si la recherche de vérification de la singularité retourne un score `> verification_beta`, cela signifie qu'un autre coup est presque aussi bon. Le `tt_move` n'est pas si spécial. On peut le pénaliser avec une réduction de -1.

## 🔧 **Comment ça marche**

Intégrer cette logique dans l'implémentation des Singular Extensions.

```c
if (second_best_score < verification_beta) { // Coup singulier
    int extension = 1;
    if (gives_check(board, &tt_move)) {
        extension++; // Double extension
    }
    // ...
} else { // Pas si singulier
    int reduction = -1; // Negative extension
    // ...
}
```

## 📊 **Gains Attendus**

- Inclus dans le gain des Singular Extensions, apporte de la robustesse et de la précision à la recherche.

---

# 💨 **15. QS Futility Pruning (Delta Pruning)**

## 🧠 **Concept**

**Idée** : Appliquer une forme de futility pruning à la recherche de quiétude (Quiescence Search).

**Intuition** : Dans la QS, on ne cherche que les captures. Si notre évaluation actuelle (`stand_pat`) plus la valeur de la pièce capturée est **toujours inférieure à alpha**, alors cette capture est futile et ne pourra jamais améliorer notre score.

**Remarque** : Cette technique est déjà présente dans votre code sous le nom de **Delta Pruning** ! C'est une excellente nouvelle.

## 🔧 **Comment ça marche (votre implémentation actuelle)**

```c
// Dans quiescence_search_depth()
int stand_pat = evaluate_position(board);
// ...

for (int i = 0; i < ordered_captures.count; i++) {
    // Delta pruning - ignorer les captures très faibles
    int delta = piece_value(ordered_captures.moves[i].captured_piece) + 200; // 200 = marge de sécurité
    if (stand_pat + delta < alpha) {
      continue; // Élague cette capture
    }
    // ...
}
```

## 📊 **Gains Attendus**

- **Gain ELO** : **+15 à +30 ELO**.
- Réduit significativement le nombre de captures inutiles explorées en fin de recherche.

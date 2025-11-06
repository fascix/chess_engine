# 🎯 Plan d'Implémentation Optimal - 15 Techniques Avancées

## 📊 Ordre d'Implémentation (Impact ELO décroissant + Dépendances)

### 🔥 **Phase 1 : Fondations (Impact: +270-380 ELO)**

#### **1. Null Move Pruning (NMP)** - ✅ PRIORITÉ ABSOLUE

- **Gain:** +60-100 ELO
- **Complexité:** Moyenne
- **Prérequis:** Aucun (fondamental)
- **Où:** Dans `negamax_alpha_beta()`, avant la génération des coups
- **Conditions critiques:**
  - `depth >= 3`
  - `!is_in_check()`
  - `has_non_pawn_material()`
  - Pas dans une séquence de mat

#### **2. Late Move Reductions (LMR)** - ✅ TRÈS IMPACTANT

- **Gain:** +80-120 ELO
- **Complexité:** Moyenne-Haute
- **Prérequis:** Bon move ordering (déjà OK dans votre code)
- **Où:** Dans la boucle de coups de `negamax_alpha_beta()`
- **Formule:** `reduction = log(depth) * log(move_index)`

#### **3. Reverse Futility Pruning (RFP)**

- **Gain:** +20-40 ELO
- **Complexité:** Faible
- **Où:** Avant génération des coups, depth <= 7

#### **4. Futility Pruning**

- **Gain:** +30-50 ELO
- **Complexité:** Faible
- **Où:** Dans la boucle de coups quiets, depth <= 4

---

### 🚀 **Phase 2 : Optimisations Move Ordering (+90-150 ELO)**

#### **5. Butterfly History Heuristic**

- **Gain:** +10-20 ELO
- **Complexité:** Faible
- **Impact:** Prépare le terrain pour LMR et History Pruning
- **Changement:** `history_scores[64][64]` au lieu de `[2][64][64]`

#### **6. Late Move Pruning (LMP)**

- **Gain:** +40-60 ELO
- **Complexité:** Faible
- **Où:** Dans la boucle de coups, après N coups testés

#### **7. Improving Heuristic**

- **Gain:** +20-40 ELO
- **Complexité:** Moyenne
- **Impact:** Améliore TOUS les prunings précédents
- **Mécanisme:** Comparer `eval` avec `eval_stack[ply-2]`

---

### ⚡ **Phase 3 : Techniques Avancées (+140-260 ELO)**

#### **8. Internal Iterative Reductions (IIR)**

- **Gain:** +15-30 ELO
- **Complexité:** Moyenne
- **Où:** PV nodes sans hash move valide

#### **9. Continuation History (CMH + FMH)**

- **Gain:** +30-50 ELO
- **Complexité:** Haute
- **Nouvelles structures:** Tables 4D pour paires de coups

#### **10. Capture History Heuristic**

- **Gain:** +20-40 ELO
- **Complexité:** Moyenne
- **Complète:** MVV-LVA avec données historiques

#### **11. History Pruning**

- **Gain:** +20-30 ELO
- **Complexité:** Faible
- **Prérequis:** Butterfly History (#5)

---

### 🎖️ **Phase 4 : Techniques Expert (+70-140 ELO)**

#### **12. Singular Extensions**

- **Gain:** +50-100 ELO
- **Complexité:** Très Haute
- **Mécanisme:** Recherche de vérification pour détecter les coups "singuliers"
- **Inclut:** Double/Triple/Negative Extensions

#### **13. Multicut**

- **Gain:** +20-40 ELO
- **Complexité:** Moyenne
- **Synergie:** Fonctionne avec les autres prunings

---

### ✅ **Déjà Implémenté**

#### **14. QS Futility Pruning (Delta Pruning)**

- **Statut:** ✅ Déjà dans votre code (ligne ~520 de search.c)
- **Implémentation:** `if (stand_pat + delta < alpha) continue;`
- **Gain estimé:** +15-30 ELO (déjà acquis)

---

## 🎯 **Ordre de Priorité Recommandé**

### Début Immédiat (Impact Max):

1. **Null Move Pruning** → +60-100 ELO (1-2h)
2. **Late Move Reductions** → +80-120 ELO (2-3h)
3. **RFP + Futility** → +50-90 ELO (1h)

### Optimisations Rapides:

4. **Butterfly History** → +10-20 ELO (30min)
5. **LMP + Improving** → +60-100 ELO (1-2h)

### Avancées:

6. **IIR + History Pruning** → +35-60 ELO (1-2h)
7. **Continuation History** → +30-50 ELO (3-4h)
8. **Capture History** → +20-40 ELO (1h)

### Expert:

9. **Singular Extensions** → +50-100 ELO (4-6h)
10. **Multicut** → +20-40 ELO (1h)

---

## 📈 **Gain Total Estimé**

- **Minimum:** +435 ELO
- **Maximum:** +750 ELO
- **Médiane:** ~590 ELO

---

## 🚦 **Par où commencer ?**

Je recommande **Null Move Pruning (NMP)** car :

1. Impact immédiat (+60-100 ELO)
2. Fondation pour d'autres techniques
3. Relativement simple à implémenter
4. Très stable si bien codé

Prêt à commencer par le NMP ? 🚀

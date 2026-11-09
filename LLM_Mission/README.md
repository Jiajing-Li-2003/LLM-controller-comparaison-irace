# LLM_Mission — comparaison appariée irace vs LLM sur 20 missions variables

Place ce dossier **n'importe où dans le dépôt** — à la racine
(`mandarina-supp/LLM_Mission/`) ou dans `software/` — puis lance **un seul
script**. Les scripts retrouvent la racine du dépôt en remontant jusqu'au
dossier qui contient `software/argos3-installation/automode/irace-files`.
Si la détection échoue (dossier déplacé hors du dépôt), force-la avec
`export MANDARINA_ROOT=/chemin/vers/le/depot`.

Il génère 20 missions aux paramètres différents, et pour chacune fait tourner
irace puis le LLM, budget 6000 chacun, `firstTest = 40`.

```
mandarina-supp-mission/
├── LLM_Mission/                 ← ce dossier (ici à la racine ; software/ marche aussi)
└── software/
    └── argos3-installation/     ← dépôt de base, jamais modifié par les scripts Python
        ├── setup_base.sh
        ├── automode_llm_common_missions.py
        ├── generate_missions.py
        ├── run_paired_comparison_d_missions.py
        ├── extract_iteration_results.R
        ├── missions/            ← généré : les 20 instances .argos par tâche
        ├── runs/                ← généré : logs irace, .Rdata, TSV
        └── resultats/           ← généré : rapports + récapitulatif
```

---

## 1. Ce qu'il faut faire au dossier de base

Le dépôt de base n'est pas utilisable tel quel. Trois choses manquent, et
`setup_base.sh` s'occupe des trois.

**a) La mission `foraging` n'existe que sous forme d'archive.**
`irace-files/` contient `irace-aggregation/` et `irace-stop/` en clair, mais
`foraging` uniquement dans `irace-foraging.tar.gz`. Sans déballage, la tâche est
inutilisable.

**b) Les chemins absolus sont faux.** Les `.argos` et les `target-runner`
pointent vers l'arborescence de leurs auteurs :

| Fichiers | Chemin inscrit |
|---|---|
| aggregation, stop | `/home/ubuntu/mandarina-supp/...` |
| foraging | `/home/dgarzon/AutoMoDe-tuttifrutti/...` et `/home/dgarzon/experiments-loop-functions/...` |

Tant qu'ils ne sont pas recalés, ARGoS ne trouve ni `libautomode.so` ni les loop
functions, et le `target-runner` renvoie `1e10` pour tout.

**c) `irace-foraging/execution-folder` n'existe pas.**

```bash
cd mandarina-supp-mission/LLM_Mission
bash setup_base.sh
```

**d) La grammaire ne borne pas les destinations de transition.** Elle déclare
`N0x0 "--n0x0 " i (0,3)` quel que soit le nombre d'états, alors que la
destination est un indice *parmi les autres* états : avec `--nstates 2` la
seule valeur valide est `0`. irace tire donc régulièrement 1, 2 ou 3, et
AutoMoDe répond `[FATAL] Could not create the Finite State Machine: Error while
parsing.` Le fichier `grammars/sq-depend-config.txt` déclare pourtant la bonne
borne (`N0x0 (|NumStates-2)`), mais aucun scénario ne le charge — irace
l'ignore. `setup_base.sh` insère le bornage dans les `target-runner`.

**e) `irace-aggregation` passe la FSM entre guillemets.** Son `target-runner`
écrit `--fsm-config "${FSM_CONFIG}"`, ce qui livre toute la machine à
`automode_main` comme **un seul argument**. Le constructeur de FSM ne trouve
alors aucun jeton et lève `vector::_M_range_check`. Les `target-runner` de
`stop` et `foraging` passent la variable sans guillemets et fonctionnent.

**f) `irace-stop` charge la mauvaise loop function.** Dans l'archive de base,
`irace-stop/experiments-folder/sequence-1.argos` charge
`libtutti_ag_loopfunc.so` tout en demandant le label `tutti_st_loop_function` —
la bibliothèque chargée n'enregistre pas ce label. Selon ce qui traîne dans
`ARGOS_PLUGIN_PATH`, ARGoS refuse de démarrer ou résout le label ailleurs ;
dans les deux cas la mission `stop` ne mesure pas ce qu'elle devrait.

Le script affiche ensuite un bilan complet, et surtout un **test de fumée** : il
exécute réellement une FSM triviale via le `target-runner` de chaque mission,
comme le fera irace. Tant que ce test ne renvoie pas un score, ne lance pas la
campagne — irace tournerait en renvoyant `1e10` pour toutes les configurations
sans que rien ne le signale.

`setup_base.sh` est **idempotent** : le relancer ne casse rien, et il répare un
dépôt dont les chemins ont été abîmés.

**À part ça, il faut compiler** — `setup_base.sh` ne le fait pas. Suis
`argos3-installation/installation-instructions.txt` dans l'ordre : ARGoS3 v48,
plugin e-puck, plugin arena, DAO e-puck, loop-functions, puis AutoMoDe. Attention,
ce fichier écrit des chemins en `~/supp-mandarina/mandarina-supp/...` : adapte-les
à ton emplacement réel.

Enfin, dans R : `install.packages("irace")`.

Si tu as sorti `LLM_Mission/` du dépôt :
```bash
export MANDARINA_ROOT=/chemin/vers/mandarina-supp-mission
```

---

## 2. Lancer

```bash
export ANTHROPIC_API_KEY=...
python3 run_paired_comparison_d_missions.py aggregation
```

C'est tout. Au premier appel, le script génère les 20 missions, puis enchaîne
pour chacune : étape 0 (pool aléatoire commun) → branche A (irace) → branche B
(LLM), et écrit un rapport. À la fin, un récapitulatif avec test de Wilcoxon
apparié sur les 20 paires.

Pour inspecter le tirage avant de lancer quoi que ce soit :
```bash
python3 generate_missions.py            # affiche le tableau des 3 tâches
```

### Découper le travail

20 missions × 2 branches × 6000 évaluations = **240 000 simulations ARGoS par
tâche**. En séquentiel, compte plusieurs jours. Le script est **reprenable** :
une mission dont le rapport existe déjà est sautée.

```bash
python3 run_paired_comparison_d_missions.py aggregation --missions 1-5    # en parallèle
python3 run_paired_comparison_d_missions.py aggregation --missions 6-10
python3 run_paired_comparison_d_missions.py aggregation --missions 11-20
python3 run_paired_comparison_d_missions.py aggregation --missions 1-20   # récap final
```

Autres options : `--redo` (refaire une mission déjà traitée), `--regen`
(retirer les 20 missions — change le banc, à éviter en cours de campagne).

### Contenu d'un rapport

Chaque `resultats/PAIRED_COMPARISON_<tâche>_mission-NN.txt` contient les
paramètres de l'instance, les deux scores finaux, les deux FSM gagnantes, puis
**deux historiques** :

- `HISTORIQUE — BRANCHE A` : une ligne par itération d'irace (best-so-far, ID de
  la configuration élite, nbConfigurations, évaluations cumulées), reconstruite
  depuis `runs/<tâche>/<mission>/irace_A.log` ;
- `HISTORIQUE — BRANCHE B` : une ligne par round du LLM (meilleur du round,
  origine du champion, nbConfigurations, taille du pool LLM, budget).

Les deux tables ont le même nombre de lignes (10), donc les trajectoires de
convergence se comparent directement.

### Les journaux complets

En plus du rapport, chaque mission dépose dans `resultats/` **un journal par
branche**, en `.txt` :

| Fichier | Contenu |
|---|---|
| `<TÂCHE>_mission-NN_irace.txt` | résumé par itération, puis le log intégral d'irace pour les 10 itérations de la branche A |
| `<TÂCHE>_mission-NN_LLM.txt` | les 10 rounds de la branche B : pour chacun, ce que le LLM a proposé (relances comprises), puis le log irace qui l'a évalué ; le prompt système est reproduit à la fin |

Les propositions du LLM sont relues depuis `runs/<tâche>/<mission>/etat_B.json`,
donc ce fichier doit être conservé jusqu'à la fin de la campagne.

Pour produire ces journaux et enrichir les rapports des missions **déjà
terminées**, sans relancer la moindre simulation :

```bash
python3 export_resultats.py aggregation
```

Relancer le script sur une mission déjà traitée ne change rien au rapport et
réécrit simplement les deux journaux.

### Figures de comparaison

```bash
python3 plot_comparaison.py aggregation
```

Produit dans `resultats/figures/` :

| Fichier | Contenu |
|---|---|
| `<tâche>_par_mission.png` | une courbe irace vs LLM par mission, en grille ; les croix marquent les rounds où le champion de B venait d'un tirage d'irace |
| `<tâche>_moyenne.png` | moyenne sur les missions, round par round, bande = intervalle de confiance bilatéral 95 % (test t) |
| `<tâche>_normalise.png` | idem après normalisation par le score du round 1 de chaque mission, pour comparer les formes de convergence malgré les échelles très différentes |
| `<tâche>_difference.png` | différence appariée B − A avec son IC 95 % ; sous la ligne zéro, le LLM fait mieux |
| `<tâche>_donnees.tsv` | les nombres, avec une colonne indiquant si l'écart est significatif |

Le script applique par défaut un **minimum courant** à la branche B, pour lui
donner la même sémantique de « meilleur connu après r rounds » que le
`best-so-far` d'irace. `--brut` désactive ce traitement et trace le meilleur de
chaque round tel quel.

Attention à l'interprétation : l'estimateur d'irace est moyenné sur beaucoup
plus d'évaluations que celui du LLM, et le minimum d'estimations bruitées est
optimiste. Le rappel figure sur les figures elles-mêmes.

### Suivre l'avancement et reprendre

```bash
python3 run_paired_comparison_d_missions.py aggregation --status
```

Affiche, mission par mission, ce qui est terminé, interrompu ou pas commencé,
et pour une mission interrompue où elle en était exactement. N'exécute rien.

La reprise est **fine** : relancer le script reprend une mission interrompue là
où elle s'est arrêtée, sans refaire l'étape 0 ni la branche A si elles étaient
finies, et sans refaire les rounds de la branche B déjà joués. L'historique de
dialogue avec le LLM est sauvegardé dans `runs/<tâche>/<mission>/etat_B.json`
après chaque round, donc un Ctrl+C au round 8 ne coûte qu'un round, pas 6000
évaluations.

---

## 3. Ce qui varie d'une mission à l'autre

Le tirage est reproductible (graine 20260914 dans `generate_missions.py`) et
**identique pour les deux branches** : elles travaillent sur le même fichier
`.argos`.

| Paramètre | Plage | Publié |
|---|---|---|
| `number_robots` (+ `<entity quantity>`) | 10 – 30, **plafonné à 12 robots/m²** | 20 |
| `lenght_boxes` (échelle de l'arène) | propre à la tâche, **jamais au-dessus de la valeur publiée** | 0.25 |
| demi-côté de `<position uniform>` | 0.35·R à 1.00·R, plancher anti-collision | 1.0·R |
| `range` du `epuck_range_and_bearing` | 0.30 – 0.80 m | 0.50 |

Deux notions de distance entre robots sont couvertes : la **dispersion initiale**
et la **portée de perception mutuelle**. La seconde est celle qui change le sens
des conditions 3 et 4 (comptage de voisins) — c'est là qu'un LLM capable de
raisonner peut se démarquer, et le prompt lui donne explicitement la valeur de
cette portée pour la mission courante.

### Pourquoi `number_edges` et `number_boxes_per_edge` ne varient pas

Ce n'est pas de la prudence, c'est une contrainte du code. Les loop functions
colorent murs et blocs **par indice codé en dur**, et `SetWallColor` /
`SetBoxColor` utilisent `std::vector::at()` : un indice hors bornes lève une
exception et fait planter ARGoS.

| Tâche | edges | boxes | R publié | Contrainte |
|---|---|---|---|---|
| aggregation | 6 | 4 | 0.866 | `SetWallColor(1,2,4,5)` ; et avec 6 côtés les murs 1 et 2 sont à 60° et 120°, donc **en haut**, face à la zone récompensée `Y ≥ 0.465`. Changer le nombre de côtés casse cette correspondance. |
| stop | 8 | 3 | 0.905 | `m_unStopEdge = Uniform(1,9)` → jusqu'à 8 : il faut **exactement 8 côtés**. `m_unStopBox = Uniform(1,4)` → au moins 3 blocs. |
| foraging | 4 | 6 | 0.750 | `SetBoxColor(1..6, mur 4)` → au moins 6 blocs et 4 côtés. |

Chaque tâche a donc sa propre géométrie dans le dépôt de base. Seule l'échelle
peut bouger, et dans une fenêtre étroite, parce que les zones de récompense sont
elles aussi en coordonnées absolues :

| Tâche | R = f(L) | plage de L | plage de R | R publié |
|---|---|---|---|---|
| aggregation | 3.4641·L | 0.217 – 0.250 | 0.752 – 0.866 | 0.866 |
| stop | 3.6213·L | 0.200 – 0.250 | 0.724 – 0.905 | 0.905 |
| foraging | 3.0000·L | 0.225 – 0.250 | 0.675 – 0.750 | 0.750 |

L'arène ne peut que **rétrécir**. ARGoS borne l'espace à `<arena size="2,2">`,
soit ±1.0 m, et le bloc le plus excentré d'un mur se trouve à la distance
`sqrt(R² + ((blocs−1)·L/2)²)`. Les géométries publiées sont déjà à 0.94–0.98 m
de limite : les agrandir fait lever
`CGrid<ENTITY>::PositionToCell() : Position <...> out of bounds` dès le premier
pas de temps. `generate_missions.py` vérifie chaque instance en l'exécutant
réellement une fois (`--no-verify` pour sauter cette étape).

### Le rayon de distribution des robots — patch C++ requis

`SeqLoopFunction::GetRandomPosition()` contient, dans le dépôt publié :

```cpp
m_fDistributionRadius = 0.4;    // en dur
```

Cette ligne **écrase** la valeur que `Init()` venait de calculer
(`m_fDistributionRadius = GetArenaRadious()`). Deux conséquences : le bloc
`<distribute>` du `.argos` est sans effet, puisque `MoveRobots()` replace
chaque robot au démarrage ; et plus l'arène est petite, plus le paquet de
robots démarre près du mur. À R = 0.733 il ne reste que 5 cm, le moteur 2D
pousse un robot à travers la paroi et ARGoS lève
`CGrid<ENTITY>::GetEntitiesAt() : Position <...> out of bounds`.

```bash
python3 patch_distribution_radius.py     # puis recompiler les loop functions
```

Le rayon devient une fraction du rayon interne, lue dans le `.argos` :

```xml
<params ... distribution_radius_ratio="0.45" />
```

Par défaut 0.45, ce qui reproduit presque exactement le comportement publié aux
tailles d'arène publiées. **À déclarer dans le mémoire : c'est une modification
du code publié.**

Sans ce patch, `distribution_radius_ratio` est ignoré et les petites arènes
plantent.

Pour ouvrir vraiment la variation d'arène il faudrait, côté C++, rendre les
seuils proportionnels à `InnerRadious()` et remplacer les indices de murs par un
calcul angulaire. Chantier à part, à réserver à la phase « Chocolate ».

---

## 4. Ce qui change par rapport à tes fichiers d'origine

La structure et le prompt sont les mêmes. Quatre corrections de fond.

### a) Les descriptions de mission

Elles sont maintenant dérivées ligne par ligne des loop functions C++, avec les
références en commentaire dans `automode_llm_common_missions.py`. C'est le seul texte qui
peut donner au LLM un avantage sur l'échantillonnage aveugle d'irace.

**`stop`** — ce n'est pas « s'arrêter ». `TuttiStLoopFunc.cpp:137-148` tire un
instant secret dans [400, 600[ ; **avant**, le score pénalise chaque robot
*immobile* ; **après**, chaque robot *en mouvement*. À cet instant, un bloc de
mur tiré au hasard passe au **vert**. La mission est : bouger, guetter le vert,
s'arrêter. L'ancienne description disait l'inverse, et ça expliquait la défaite
significative de la branche B sur cette tâche.

**`aggregation`** — la zone récompensée est `Y ≥ 0.465`, mais le sol est **noir
dans les deux bandes**, haute et basse. Seule la haute rapporte. Les murs lèvent
l'ambiguïté : murs 1 et 2 **bleus** en haut, murs 4 et 5 **verts** en bas. Un
contrôleur qui ne réagit qu'au sol noir envoie la moitié du groupe du mauvais
côté — ce que faisait le tien, avec `BlackFloor` utilisé 31 fois sur 30 runs.

**`foraging`** — les deux moitiés de la source ont des valeurs **opposées** :
livrer un objet pris au sud fait −1, au nord +1. Une récolte non discriminante
marque donc environ zéro.

### b) `--b` / `--bi` n'existent pas

`AutoMoDeFsmBuilder.cpp:223` n'accepte que `{"p","w","l"}` pour les conditions.
Pour les conditions 3 et 4, le seuil de voisins passe par **`--p<I>x<K>`**, en
entier. L'ancien catalogue documentait `--b<I>x<K>` : une FSM écrite ainsi
levait `THROW_ARGOSEXCEPTION("Missing Parameter")`, et le pont Python rangeait
la probabilité écrite par le LLM dans le champ « seuil » — d'où les
`Parameter '--b0x1' was generated as 0.8` qui polluaient ensuite tous les
prompts. La règle 5.3 du prompt le dit maintenant explicitement.

Vérifié : le pont rejoue sans une seule erreur une FSM publiée du corpus TF-WS,
avec `B0x1=3`, `B2x0=7`, `BI2x1=4` correctement placés.

### c) Le budget caché est supprimé

Le pré-filtrage par simulation est conservé (il évite de gaspiller le budget
irace sur des FSM qui plantent), mais ses simulations sont **décomptées du
budget de la branche B**. L'ancienne version en dépensait jusqu'à 1620 hors
budget, soit +27 % pour la seule branche LLM.

### d) Les deux branches jouent sous les mêmes contraintes

L'ancien filtre « graphe fortement connexe » ne s'appliquait qu'à B, alors
qu'irace y échappait et que les FSM publiées de TuttiFrutti comportent des
machines à un seul état. `validate_topology` ne rejette plus que ce qui fait
réellement planter AutoMoDe.

### e) Détails

- Le seuil « score trop mauvais » n'est plus la constante 23999 mais
  `robots × 1200 − 1`, recalculé par mission — nécessaire puisque le nombre de
  robots varie. Avec 20 robots il vaut exactement 23999, ce qui confirme au
  passage l'origine de l'ancienne constante.
- La **provenance** du champion de la branche B est tracée à chaque round : le
  rapport indique s'il vient d'une proposition du LLM ou d'un tirage d'irace.
  Sans ça, une victoire de B n'est pas interprétable — irace complète le pool
  quand le LLM fournit moins de configurations que `nbConfigurations`.
- Le TSV d'extraction porte une colonne `NEval` : le nombre d'évaluations sur
  lequel chaque moyenne est calculée.

---

## 5. Une limite à garder en tête

Le récapitulatif compare des **scores d'entraînement**. L'élite d'irace est
moyennée sur beaucoup plus d'évaluations que le meilleur d'un round LLM, et
prendre le minimum de moyennes bruitées introduit un biais optimiste qui joue en
faveur de B. La colonne `NEval` te permet de voir l'ampleur de l'écart.

Pour un verdict publiable, il faudra rejouer les deux champions finaux sur des
graines de test disjointes de l'apprentissage, une trentaine par mission, et
refaire le test apparié là-dessus. Le script affiche ce rappel à la fin.

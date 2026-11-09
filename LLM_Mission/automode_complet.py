#!/usr/bin/env python3
"""
automode_complet.py

Variante "classement complet" du module partage.

Deux differences avec automode_llm_common_missions.py, et deux seulement :

  1. PAS DE BANC DE MISSIONS. On tourne 10 fois de suite sur l'instance
     PUBLIEE de la tache, sans faire varier ses parametres. Les repetitions
     ne different que par la graine, ce qui est le protocole AutoMoDe usuel.

  2. Les sorties vont dans resultats_complet/ et runs_complet/, pour ne pas
     se melanger aux autres campagnes.

Le prompt systeme, les descriptions de mission, les catalogues de modules et
la conversion CLI sont IDENTIQUES. Le changement de retour au LLM (classement
complet au lieu de 5 meilleurs / 3 pires) est dans le runner.
"""

import math
import os
import re
import subprocess
import sys

# ===========================================================================
# 1. CHEMINS
# ===========================================================================
LLM_DIR = os.path.dirname(os.path.abspath(__file__))

_MARKER = os.path.join("software", "argos3-installation", "automode", "irace-files")


def _find_mandarina_root(start: str):
    """Remonte depuis `start` jusqu'a trouver la racine du depot.

    Fonctionne que LLM_Mission/ soit place a la racine du depot
    (mandarina-supp/LLM_Mission) ou dans software/ (mandarina-supp/software/LLM_Mission).
    """
    d = start
    while True:
        if os.path.isdir(os.path.join(d, _MARKER)):
            return d
        parent = os.path.dirname(d)
        if parent == d:
            return None
        d = parent


MANDARINA = os.environ.get("MANDARINA_ROOT") or _find_mandarina_root(LLM_DIR)

if MANDARINA is None:
    print("[ERREUR] Racine du depot introuvable en remontant depuis :")
    print(f"           {LLM_DIR}")
    print("         Aucun dossier parent ne contient")
    print(f"           {_MARKER}")
    print()
    print("         Corrige avec :")
    print("           export MANDARINA_ROOT=/chemin/vers/mandarina-supp-mission")
    sys.exit(1)

BASE = f"{MANDARINA}/software/argos3-installation/automode"
EXE = f"{BASE}/bin/automode_main"

# pour les 10 repetitions.
RESULTS_DIR = os.path.join(LLM_DIR, "resultats_complet")   # rapports
RUNS_DIR = os.path.join(LLM_DIR, "runs_complet")           # logs, .Rdata, TSV

TASKS = ["aggregation", "stop", "foraging"]

ANTHROPIC_API_KEY = os.environ.get("ANTHROPIC_API_KEY")
MODEL = "claude-sonnet-4-6"
TEMPERATURE = 0.9

EVAL_SEEDS = [12345, 42, 999]
ERROR_MEMORY_FILE_TEMPLATE = os.path.join(LLM_DIR, "llm_errors_memory_{task}.txt")


def grammar_file(task: str) -> str:
    return f"{BASE}/irace-files/irace-{task}/grammars/sq-grammar.txt"


def target_runner(task: str) -> str:
    return f"{BASE}/irace-files/irace-{task}/target-runner"


def reference_argos(task: str) -> str:
    """Instance publiee, gabarit des 20 missions generees."""
    return f"{BASE}/irace-files/irace-{task}/experiments-folder/sequence-1.argos"


def check_api_key():
    if not ANTHROPIC_API_KEY:
        print("[ERREUR] La variable d'environnement ANTHROPIC_API_KEY n'est pas definie.")
        sys.exit(1)


def check_base():
    problems = []
    if not os.path.isfile(EXE):
        problems.append(f"automode_main introuvable : {EXE}")
    for t in TASKS:
        if not os.path.isfile(grammar_file(t)):
            problems.append(f"grammaire manquante : {grammar_file(t)}")
        if not os.path.isfile(reference_argos(t)):
            problems.append(f"instance de reference manquante : {reference_argos(t)}")
    if problems:
        print("[ERREUR] Le depot de base n'est pas pret :")
        for p in problems:
            print("  - " + p)
        print("\n  Lance ./setup_base.sh, et verifie MANDARINA_ROOT.")
        sys.exit(1)


# ===========================================================================
# 2. MISSIONS : geometrie et description
# ===========================================================================
# arena_entity.cpp:200 — InnerRadious() = lenght_boxes * boxes / (2*tan(pi/edges))
def inner_radius(edges: int, boxes: int, length: float) -> float:
    return (length * boxes) / (2 * math.tan(math.pi / edges))


# ---------------------------------------------------------------------------
# POURQUOI number_edges ET number_boxes_per_edge NE SONT PAS TIRES AU HASARD
# ---------------------------------------------------------------------------
# Les loop functions colorent murs et blocs par INDICE CODE EN DUR, et
# CArenaEntity::SetWallColor / SetBoxColor utilisent std::vector::at() :
# un indice hors bornes leve une exception et fait planter ARGoS.
#
#   aggregation : SetWallColor(1,2 -> BLEU ; 4,5 -> VERT).  Avec 6 cotes, le mur
#                 i est centre a l'angle 60*i : murs 1 et 2 en HAUT, murs 4 et 5
#                 en BAS. Changer le nombre de cotes deplace ces murs et casse
#                 leur correspondance avec la zone recompensee Y >= 0.465.
#   stop        : m_unStopEdge = Uniform(1,9) -> 1..8  => il faut 8 cotes
#                 m_unStopBox  = Uniform(1,4) -> 1..3  => il faut >= 3 blocs
#   foraging    : SetBoxColor(1..6, mur 4)             => >= 6 blocs, >= 4 cotes
#
# Seule l'echelle (lenght_boxes) peut bouger, et dans une fenetre etroite : les
# zones de recompense sont elles aussi en coordonnees absolues.
# ---------------------------------------------------------------------------

# La longueur ne varie QUE VERS LE BAS, jamais au-dessus de la valeur publiee.
# Raison : ARGoS borne l'espace a <arena size="2,2">, soit +-1.0 m, et le bloc
# le plus excentre d'un mur se trouve a la distance
#     sqrt(R_in^2 + ((blocs-1) * longueur / 2)^2)
# Au-dela de 1.0 m, la grille des LED leve
#     CGrid<ENTITY>::PositionToCell() : Position <...> out of bounds
# des le premier pas de temps. Les geometries publiees sont a 0.94-0.98 m,
# donc deja proches de la limite : les agrandir n'est pas possible sans
# agrandir aussi <arena size>, ce qui changerait la mission publiee.
ARENA_SPEC = {
    # (cotes, blocs par cote, longueur publiee, plage de longueur exploitable)
    # Meme plage relative pour les trois taches : R varie de 0.87 a 1.00 fois
    # le rayon publie, soit exactement ce qu'a subi la campagne aggregation.
    "aggregation": dict(edges=6, boxes=4, length_published=0.25, length_range=(0.217, 0.250)),
    "stop":        dict(edges=8, boxes=3, length_published=0.25, length_range=(0.217, 0.250)),
    "foraging":    dict(edges=4, boxes=6, length_published=0.25, length_range=(0.217, 0.250)),
}


def outermost_block_distance(edges: int, boxes: int, length: float) -> float:
    """Distance du bloc le plus excentre au centre. Doit rester sous 1.0 m."""
    return math.hypot(inner_radius(edges, boxes, length), (boxes - 1) * length / 2)

# Plages communes aux trois missions
ROBOTS_RANGE = (10, 30)              # publie : 20

# Densite maximale de robots, en robots/m2 de surface d'arene.
#
# Sans plafond, le tirage combine "beaucoup de robots" et "petite arene" :
# stop mission-01 tombait a 28 robots dans une arene de rayon 0.733, soit
# 16.6 robots/m2 contre 7.8 pour la configuration publiee. Le moteur 2D finit
# par pousser un robot a travers un mur, et ARGoS leve
#     CGrid<ENTITY>::GetEntitiesAt() : Position <...> out of bounds
# des la premiere configuration. Les densites publiees vont de 7.8 a 11.3 ;
# 12 laisse de la marge au-dessus sans approcher la zone de casse.
MAX_DENSITY = None   # plus de plafond : le ratio de distribution borne le tassement
# Fraction du rayon interne de l'arene dans laquelle MoveRobots() place les
# robots. Lue par le C++ APRES application de patch_distribution_radius.py.
# Sans ce patch, SeqLoopFunction::GetRandomPosition() fige le rayon a 0.4 et
# ce parametre n'a aucun effet.
DISTRIB_RATIO_RANGE = (0.30, 0.55)   # 0.45 reproduit le comportement publie
RAB_RANGE = (0.30, 0.80)             # portee range-and-bearing ; publie : 0.50


# ---------------------------------------------------------------------------
# DESCRIPTIONS DE MISSION : DES FAITS, PAS DES SOLUTIONS
# ---------------------------------------------------------------------------
# Chaque description dit ce qu'est l'environnement et comment le score est
# calcule, et rien de plus. Elle ne dit PAS quels modules combiner : ce serait
# donner la solution au LLM, alors qu'irace doit la trouver seul. Les principes
# de conception generaux, identiques pour toutes les missions, sont dans le
# prompt systeme (section "HOW TO APPROACH THE DESIGN").
#
# Les references de ligne renvoient aux loop functions qui definissent ces faits.
# ---------------------------------------------------------------------------
MISSION_DESCRIPTIONS = {

    # TuttiStLoopFunc.cpp:137-148 (score), :140-142 (signal), :154-184 (comptage)
    "stop": (
        "The score is MINIMISED.\n"
        "\n"
        "Each run is divided into two phases by a switch time drawn uniformly at random between "
        "400 and 600 simulation steps. The robots are not told when it occurs.\n"
        "\n"
        "Before the switch time, at every step, the score increases by 1 for every robot that is "
        "stationary. From the switch time onwards, at every step, the score increases by 1 for every "
        "robot that is moving. A robot counts as moving if it travelled more than 0.0005 m since the "
        "previous step.\n"
        "\n"
        "At the switch time, one wall block of the arena, chosen at random, lights up GREEN. The "
        "floor of the arena is uniformly GRAY."
    ),

    # TuttiAgLoopFunc.cpp:176-180 (zone), :163-169 (sol), :122-128 (murs)
    "aggregation": (
        "The score is MINIMISED.\n"
        "\n"
        "At every step, the score increases by 1 for every robot that is outside the target region. "
        "The score does not depend on how close the robots are to each other.\n"
        "\n"
        "The arena has two strips at opposite ends. The floor is BLACK in both strips and GRAY in "
        "between. The target region is the strip whose bordering walls are BLUE; the walls bordering "
        "the other strip are GREEN."
    ),

    # TuttiFrLoopFunc.cpp:150-190 (score), :193-206 (sol), :120-130 (blocs)
    "foraging": (
        "The score is MINIMISED.\n"
        "\n"
        "The arena contains a nest area at one end, with a WHITE floor, and a source area at the "
        "opposite end, with a BLACK floor. The floor in between is GRAY. The wall on the nest side "
        "is RED.\n"
        "\n"
        "The source area is divided into two halves. The wall blocks behind one half are BLUE and "
        "those behind the other half are GREEN.\n"
        "\n"
        "A robot carries at most one item. Entering a half of the source gives the robot an item of "
        "that half's type, replacing any item it was already carrying. Entering the nest with an "
        "item delivers it: an item from one half decreases the score by 1, an item from the other "
        "half increases it by 1."
    ),
}


# ===========================================================================
# 3. CATALOGUE DES MODULES (binaire AutoMoDe de base)
# ===========================================================================
BEHAVIOR_CATALOG = {
    "0":  ("Exploration",    "moves forward and turns randomly",
           [("RWM{i}", "--rwm<I>", "turning rate")]),
    "1":  ("Stop",           "stands completely still", []),
    "2":  ("Phototaxis",     "moves toward the light source", []),
    "3":  ("AntiPhototaxis", "moves away from the light source", []),
    "4":  ("Attraction",     "moves toward nearby robots",
           [("ATT{i}", "--att<I>", "attraction gain")]),
    "5":  ("Repulsion",      "moves away from nearby robots",
           [("REP{i}", "--rep<I>", "repulsion gain")]),
    "6":  ("Spiral",         "traces an expanding spiral",
           [("SPD{i}", "--spd<I>", "spiral speed")]),
    "8":  ("GoToColor",      "moves toward the nearest object of a target colour",
           [("VEL{i}", "--vel<I>", "speed"), ("CLR{i}", "--clr<I>", "target colour")]),
    "9":  ("GoAwayColor",    "moves away from the nearest object of a target colour",
           [("VEL{i}", "--vel<I>", "speed"), ("CLR{i}", "--clr<I>", "target colour")]),
    "10": ("FlockCenter",    "moves toward the centre of the group",
           [("FC{i}", "--fc<I>", "flocking gain")]),
    "11": ("ZigZag",         "moves in a zigzag pattern",
           [("ZFR{i}", "--zfr<I>", "zigzag frequency")]),
}

CONDITION_CATALOG = {
    "0": ("floor under the robot is BLACK",
          [("P{ix}", "--p<I>x<K>", "probability, real in [0,1]")]),
    "1": ("floor under the robot is GRAY",
          [("P{ix}", "--p<I>x<K>", "probability, real in [0,1]")]),
    "2": ("floor under the robot is WHITE",
          [("P{ix}", "--p<I>x<K>", "probability, real in [0,1]")]),
    "3": ("number of neighbours ABOVE a threshold",
          [("B{ix}", "--p<I>x<K>", "threshold: an INTEGER count of neighbours, not a probability"),
           ("W{ix}", "--w<I>x<K>", "steepness of the sigmoid")]),
    "4": ("number of neighbours BELOW a threshold",
          [("BI{ix}", "--p<I>x<K>", "threshold: an INTEGER count of neighbours, not a probability"),
           ("WI{ix}", "--w<I>x<K>", "steepness of the sigmoid")]),
    "5": ("fires with a fixed probability at every step",
          [("P{ix}", "--p<I>x<K>", "probability, real in [0,1]")]),
    "7": ("the robot PERCEIVES a given colour with its camera",
          [("L{ix}", "--l<I>x<K>", "colour to look for"),
           ("P{ix}", "--p<I>x<K>", "probability, real in [0,1]")]),
}

BEHAVIOR_PARAM_DEFAULTS = {"RWM": "50", "ATT": "3", "REP": "3", "SPD": "5",
                           "FC": "3", "ZFR": "5", "VEL": "1.0", "CLR": "1"}

BEHAVIOR_PREFIXES = ["s", "rwm", "att", "rep", "vel", "cle", "clr", "spd", "fc", "zfr"]


# ===========================================================================
# 4. PROMPT
# ===========================================================================
SYSTEM_PROMPT_TEMPLATE = """You are an expert in swarm robotics working with the ARGoS3 simulator and the AutoMoDe framework.
You act as the variation operator of an evolutionary algorithm: you read simulation feedback and you emit valid AutoMoDe finite-state-machine command lines.

# 1. THE MISSION
{task_description}

# 2. THE ROBOTS AND THE ARENA
{arena_description}

# 3. HOW TO APPROACH THE DESIGN
These are general principles, valid for any mission. They are not a solution to this one.
- Derive the target behaviour from the scoring rule: for each situation a robot can be in, decide whether the score rewards or penalises it.
- Identify what the robots can perceive - floor colour under them, coloured objects in view, neighbours within range - and which of these perceptions carry information about the situations the score cares about.
- Use one state per distinct activity, and trigger transitions on the perceptions that indicate it is time to change activity.

# 4. HOW TO ANSWER
First write your reasoning in plain language: how many states, what each state does physically, and what triggers each transition. Two or three sentences per machine is enough.
Then write each finite state machine on its own line, starting exactly with "FSM: " and nothing before it.

# 5. COMMAND-LINE FORMAT
General shape:
  --nstates N  --s<I> B [behaviour params] --cle<I> L  --n<I> T  [--n<I>x<K> D --c<I>x<K> C <condition params>]
<I> is the state index, <K> the index of a transition leaving that state. Both start at 0.

## A. Global
* `--nstates N` : number of states, {nstates_domain}.

## B. Behaviours — use ONLY these ids
{behaviors_doc}

Every state also takes an LED colour:
* `--cle<I> L` : {cle_domain}  (0=off, 1=red, 2=green, 3=blue, 4=yellow, 5=magenta, 6=cyan)

## C. Transitions
* `--n<I> T` : number of transitions leaving state <I>, {max_transitions_domain}.
  Up to 4 transitions may leave the same state, each with a different condition type. A state that reacts to several different situations is often stronger than a state with one transition, and random sampling produces that variety by default.
* `--n<I>x<K> D` : destination, indexed among the OTHER states only.
  With 3 states, leaving state 1 the others are [0, 2]: D=0 targets state 0, D=1 targets state 2.
  With 2 states, leaving state 0 the only other state is state 1, so D must be 0.
* `--c<I>x<K> C` : condition type — use ONLY these ids
{conditions_doc}

# 6. HARD RULES
1. Integers stay integers; real-valued parameters always carry a decimal point (1.0, not 1).
2. Every state that exists must declare `--n<I>`, and every declared transition must carry its destination, its condition type and that condition's parameters. A missing parameter crashes the simulator.
3. `--p<I>x<K>` is a PROBABILITY for conditions {prob_conditions}, but an INTEGER COUNT OF NEIGHBOURS for conditions 3 and 4. Never write 0.8 where a neighbour count is expected.
4. Never point a transition at its own state.
"""

MULTI_FSM_ADDENDUM = """
# 7. YOU ARE THE EVOLUTIONARY OPERATOR
This is a multi-round conversation on one real ARGoS3 optimisation run. The numerical optimiser is restarted from scratch at the beginning of every round, so exploration and exploitation are entirely up to you.

1. SURVIVAL (elitism): copy the current best-ever machine verbatim in your answer so its lineage is not lost.
2. EXPLOITATION: spend about half of your machines on small edits of the best performers — shift a probability, a threshold or a gain by roughly ten percent, and keep the topology.
3. EXPLORATION: spend the rest on genuinely different topologies or module combinations, to escape local optima.
4. RICHNESS: do not let every state settle on a single transition of the same condition type. Include candidates with two to four transitions per state mixing different condition types.
"""


def published_params(task: str) -> dict:
    """Parametres reels de l'instance publiee, lus dans son .argos.

    On ne les ecrit pas en dur : si quelqu'un modifie sequence-1.argos, la
    description donnee au LLM suit automatiquement.
    """
    import xml.etree.ElementTree as ET
    root = ET.parse(reference_argos(task)).getroot()
    par = root.find("./loop_functions/params")
    rab = root.find(".//actuators/epuck_range_and_bearing")
    edges = int(par.get("number_edges"))
    boxes = int(par.get("number_boxes_per_edge"))
    length = float(par.get("lenght_boxes"))
    robots = int(par.get("number_robots"))
    r = inner_radius(edges, boxes, length)
    ratio = float(par.get("distribution_radius_ratio", 0.45))
    return dict(task=task, edges=edges, boxes=boxes, length=length,
                inner_radius=round(r, 4), radius_published=round(r, 4),
                robots=robots, density=round(robots / (math.pi * r ** 2), 2),
                distrib_ratio=ratio, distrib_radius=round(1.5 * ratio * r, 3),
                marge_mur=round(r - 1.5 * ratio * r, 3),
                rab=float(rab.get("range")) if rab is not None else 0.50)


def arena_description(params: dict) -> str:
    """Texte donne au LLM sur les robots et l'arene de la mission courante.

    Volontairement sans dimensions d'arene (rayon, nombre de cotes, disque de
    depart) : irace ne les connait pas, le LLM ne doit pas en tirer avantage.
    On decrit les robots, qui sont le support du controleur.
    """
    return (
        f"The robots operate inside a closed arena bounded by walls. {params['robots']} e-puck "
        f"robots start scattered around the centre of the arena, with random positions and "
        f"orientations that change at every run. Each robot perceives its neighbours through a "
        f"range-and-bearing device with a range of {params['rab']:.2f} m: neighbour-count "
        f"conditions only count robots closer than that. A run lasts 1200 simulation steps "
        f"(120 s at 10 steps per second)."
    )


# ===========================================================================
# 5. GRAMMAIRE IRACE
# ===========================================================================
def _split_domain(domain_str: str) -> list:
    parts, cur, in_q = [], "", False
    for ch in domain_str:
        if ch == '"':
            in_q = not in_q
            cur += ch
        elif ch == "," and not in_q:
            parts.append(cur)
            cur = ""
        else:
            cur += ch
    parts.append(cur)
    return [p.strip() for p in parts]


def parse_grammar(path: str) -> list:
    params = []
    try:
        with open(path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                main, cond = (line.split("|", 1) + [None])[:2]
                m = re.match(r'^(\S+)\s+"([^"]*)"\s+([icr])\s*\(([^)]*)\)', main.strip())
                if not m:
                    continue
                name, switch, ptype, dom = m.groups()
                if ptype == "c":
                    domain = [v.strip() for v in dom.split(",")]
                else:
                    lo_s, hi_s = _split_domain(dom)
                    try:
                        lo = float(lo_s)
                    except ValueError:
                        lo = 0.0
                    try:
                        hi = float(hi_s)
                    except ValueError:
                        hi = 3.0
                    domain = (lo, hi)
                params.append({"name": name, "switch": switch.strip(), "type": ptype,
                               "domain": domain,
                               "condition": cond.strip() if cond else None})
    except FileNotFoundError:
        print(f"[ERREUR] Fichier de grammaire introuvable : {path}")
        sys.exit(1)
    return params


def load_grammar(task: str):
    g = parse_grammar(grammar_file(task))
    return g, [p["name"] for p in g], {p["name"]: p for p in g}


def describe_param(key: str, meta: dict) -> str:
    m = meta.get(key)
    if not m:
        return "?"
    if m["type"] == "c":
        if len(m["domain"]) == 1:
            return f"fixed value {m['domain'][0]}"
        return "one of {" + ",".join(m["domain"]) + "}"
    lo, hi = m["domain"]
    f = lambda v: str(int(v)) if v == int(v) else str(v)
    return f"{'integer' if m['type'] == 'i' else 'real'} in [{f(lo)}..{f(hi)}]"


def build_behaviors_doc(meta: dict) -> str:
    out = []
    for bid in sorted(meta["S0"]["domain"], key=int):
        if bid not in BEHAVIOR_CATALOG:
            continue
        label, desc, reqs = BEHAVIOR_CATALOG[bid]
        line = f"  B={bid}  {label:<14}-> {desc}"
        for key, switch, note in reqs:
            line += f"\n            requires {switch}  {describe_param(key.format(i='0'), meta)}  ({note})"
        out.append(line)
    return "\n".join(out)


def build_conditions_doc(meta: dict) -> str:
    out = []
    for cid in sorted(meta["C0x0"]["domain"], key=int):
        if cid not in CONDITION_CATALOG:
            continue
        label, reqs = CONDITION_CATALOG[cid]
        line = f"  C={cid}  {label}"
        for key, switch, note in reqs:
            line += f"\n            requires {switch}  {describe_param(key.format(ix='0x0'), meta)}  ({note})"
        out.append(line)
    return "\n".join(out)


def build_prompt_context(task: str, meta: dict, arena_params: dict) -> str:
    prob_conditions = ", ".join(c for c in sorted(meta["C0x0"]["domain"], key=int)
                                if c not in ("3", "4"))
    prompt = SYSTEM_PROMPT_TEMPLATE.format(
        task_description=MISSION_DESCRIPTIONS[task],
        arena_description=arena_description(arena_params),
        nstates_domain=describe_param("NumStates", meta),
        behaviors_doc=build_behaviors_doc(meta),
        cle_domain=describe_param("CLE0", meta),
        max_transitions_domain=describe_param("NumConnections0", meta),
        conditions_doc=build_conditions_doc(meta),
        prob_conditions=prob_conditions,
    )
    return prompt + MULTI_FSM_ADDENDUM


# ===========================================================================
# 6. MEMOIRE D'ERREURS
# ===========================================================================
def error_memory_path(task: str, work_dir: str = None) -> str:
    """Fichier de memoire d'erreurs.

    Il est PROPRE A UNE MISSION (dans son dossier runs/) et non plus partage
    par tache. Deux raisons :
      - reproductibilite : le resultat de la mission 7 ne doit pas dependre de
        ce que le LLM a rate pendant les missions 1 a 6 ;
      - parallelisme : deux processus lances sur des plages de missions
        differentes ecrivaient dans le meme fichier et s'ecrasaient.
    """
    if work_dir:
        return os.path.join(work_dir, "llm_errors.txt")
    return os.path.join(LLM_DIR, f"llm_errors_memory_{task}.txt")


def load_error_memory(task: str, work_dir: str = None) -> list:
    p = error_memory_path(task, work_dir)
    if os.path.exists(p):
        with open(p) as f:
            return [l.strip() for l in f if l.strip()]
    return []


def save_error_memory(task: str, errors: list, work_dir: str = None):
    uniq = list(dict.fromkeys(errors))[-15:]
    p = error_memory_path(task, work_dir)
    os.makedirs(os.path.dirname(p), exist_ok=True)
    with open(p, "w") as f:
        for e in uniq:
            f.write(e + "\n")


def clear_error_memory(task: str, work_dir: str = None):
    p = error_memory_path(task, work_dir)
    if os.path.exists(p):
        os.remove(p)


# ===========================================================================
# 7. CLI <-> DICTIONNAIRE IRACE
# ===========================================================================
def cli_to_irace_dict(cli: str, task: str, meta: dict, irace_params: list):
    errors = []
    tok = cli.split()
    raw, i = {}, 0
    while i < len(tok):
        if tok[i].startswith("--") and i + 1 < len(tok) and not tok[i + 1].startswith("--"):
            raw[tok[i][2:]] = tok[i + 1]
            i += 2
        else:
            i += 1

    d = {p: "NA" for p in irace_params}

    try:
        nstates = int(raw.get("nstates", "0"))
    except ValueError:
        nstates = 0
        errors.append(f"'--nstates {raw.get('nstates')}' is not an integer.")
    if nstates <= 0:
        nstates = 1
        errors.append("Missing or invalid '--nstates'. Every machine must declare it.")
    nstates = min(nstates, 4)
    d["NumStates"] = str(nstates)

    for s in range(nstates):
        for pfx in BEHAVIOR_PREFIXES:
            if raw.get(f"{pfx}{s}") is not None:
                d[f"{pfx.upper()}{s}"] = raw[f"{pfx}{s}"]

        if nstates == 1:
            continue

        nc_raw = raw.get(f"n{s}")
        if nc_raw is None:
            errors.append(f"State {s} is missing '--n{s}' (its number of outgoing transitions).")
            nc = 1
        else:
            try:
                nc = int(nc_raw)
            except ValueError:
                nc = 1
                errors.append(f"'--n{s} {nc_raw}' is not an integer.")
        nc = max(1, min(nc, 4))
        d[f"NumConnections{s}"] = str(nc)

        for k in range(nc):
            xk = f"{s}x{k}"
            dest = raw.get(f"n{xk}")
            if dest is None:
                errors.append(f"Transition '--n{xk}' is missing its destination index.")
                dest = "0"
            try:
                dv = int(float(dest))
            except ValueError:
                dv = 0
            d[f"N{s}x{k}"] = str(max(0, min(dv, nstates - 2)))

            cond = raw.get(f"c{xk}", "5")
            d[f"C{s}x{k}"] = cond
            pval = raw.get(f"p{xk}")
            w = raw.get(f"w{xk}", "5.0")
            l = raw.get(f"l{xk}", "1")

            if cond == "3":
                if pval is None:
                    errors.append(f"Condition '--c{xk} 3' needs '--p{xk}' "
                                  f"(an INTEGER neighbour count) and '--w{xk}'.")
                d[f"B{s}x{k}"], d[f"W{s}x{k}"] = pval or "3", w
            elif cond == "4":
                if pval is None:
                    errors.append(f"Condition '--c{xk} 4' needs '--p{xk}' "
                                  f"(an INTEGER neighbour count) and '--w{xk}'.")
                d[f"BI{s}x{k}"], d[f"WI{s}x{k}"] = pval or "3", w
            elif cond == "7":
                d[f"P{s}x{k}"], d[f"L{s}x{k}"] = pval or "0.5", l
            else:
                d[f"P{s}x{k}"] = pval or "0.5"

    # --- controle des domaines de la grammaire ---
    for key, val in list(d.items()):
        if val == "NA":
            continue
        m = meta.get(key)
        if not m:
            continue
        if m["type"] in ("i", "r"):
            lo, hi = m["domain"]
            try:
                fv = float(val)
            except ValueError:
                errors.append(f"'{m['switch']}' got the non-numeric value {val!r}.")
                d[key] = str(int(lo)) if m["type"] == "i" else f"{lo:.4f}"
                continue
            if fv < lo or fv > hi:
                errors.append(f"'{m['switch']}' was set to {val}, outside the allowed "
                              f"range [{lo}, {hi}].")
                fv = max(lo, min(hi, fv))
            d[key] = str(int(round(fv))) if m["type"] == "i" else f"{fv:.4f}"
        elif m["type"] == "c" and val not in m["domain"]:
            errors.append(f"'{m['switch']}' used the invalid value {val!r}. "
                          f"Allowed: {', '.join(m['domain'])}.")
            d[key] = m["domain"][0]

    # --- parametres obligatoires des comportements ---
    for i in range(nstates):
        sv = d.get(f"S{i}")
        if sv in (None, "NA"):
            continue
        entry = BEHAVIOR_CATALOG.get(sv)
        if not entry:
            continue
        for key, switch, _ in entry[2]:
            target = key.format(i=i)
            if d.get(target, "NA") == "NA":
                errors.append(f"Behaviour '--s{i} {sv}' requires "
                              f"'{switch.replace('<I>', str(i))}', which was missing.")
                d[target] = BEHAVIOR_PARAM_DEFAULTS.get(re.sub(r"\d+$", "", target), "1")
        if f"CLE{i}" in meta and d.get(f"CLE{i}", "NA") == "NA":
            errors.append(f"State {i} is missing '--cle{i}' (LED colour).")
            d[f"CLE{i}"] = meta[f"CLE{i}"]["domain"][0]

    return d, errors


def dict_to_cli(d: dict, meta: dict, irace_params: list) -> str:
    out = [f"--nstates {d.get('NumStates', '1')}"]
    for p in irace_params:
        v = d.get(p, "NA")
        if p != "NumStates" and v != "NA" and p in meta:
            out.append(f"{meta[p]['switch']} {v}")
    return " ".join(out)


def write_configurations_file(path: str, irace_params: list, rows: list):
    with open(path, "w") as f:
        f.write("\t".join(irace_params) + "\n")
        for d in rows:
            f.write("\t".join(d.get(p, "NA") for p in irace_params) + "\n")


def fix_transition_destinations(cli: str) -> str:
    """Recadre les destinations sur [0, nstates-2], comme le fait le target-runner."""
    tok = cli.split()
    nstates = 1
    for i, t in enumerate(tok):
        if t == "--nstates" and i + 1 < len(tok):
            try:
                nstates = int(tok[i + 1])
            except ValueError:
                pass
            break
    mx = max(0, nstates - 2)
    out, i = [], 0
    while i < len(tok):
        out.append(tok[i])
        if re.match(r"^--n\d+x\d+$", tok[i]) and i + 1 < len(tok):
            i += 1
            try:
                v = int(float(tok[i]))
            except ValueError:
                v = 0
            out.append(str(min(v, mx)))
        i += 1
    return " ".join(out)


def extract_all_fsm_lines(text: str) -> list:
    out = []
    for raw in text.split("\n"):
        line = re.sub(r"^[\*\-\u2022]\s*", "", raw.strip())
        line = re.sub(r"^\d+[\.\)]\s*", "", line).strip("*_ ")
        if line.startswith("FSM:"):
            out.append(line.replace("FSM:", "", 1).strip())
    return out


def validate_topology(d: dict):
    """Ne rejette que ce qui fait planter AutoMoDe ou n'a aucun sens.

    Volontairement plus permissif que l'ancien filtre "graphe fortement connexe" :
    irace n'est soumis a aucune contrainte de ce type, donc en imposer une a la
    branche LLM biaisait la comparaison. Les FSM publiees de TuttiFrutti
    comportent d'ailleurs des machines a un seul etat.
    """
    try:
        n = int(d.get("NumStates", "1"))
    except ValueError:
        return False, "NumStates illisible"
    if n < 1 or n > 4:
        return False, f"NumStates={n} hors de [1,4]"
    if n == 1:
        return True, "Valide"
    for s in range(n):
        if d.get(f"NumConnections{s}", "NA") == "NA":
            return False, f"l'etat S{s} ne declare aucune transition"
    return True, "Valide"


# ===========================================================================
# 8. EVALUATION
# ===========================================================================
def evaluate_fsm(cli: str, argos_file: str, seeds=None):
    """Moyenne du score sur quelques graines. Renvoie None si tout a echoue."""
    cli = fix_transition_destinations(cli)
    env = {**os.environ, "QT_QPA_PLATFORM": "offscreen", "DISPLAY": ""}
    scores = []
    for seed in (seeds or EVAL_SEEDS):
        cmd = [EXE, "-n", "-c", argos_file, "--seed", str(seed), "--fsm-config"] + cli.split()
        try:
            r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               text=True, timeout=120, env=env)
            m = re.search(r"Score\s+([-+]?\d+(?:\.\d+)?)", r.stdout)
            if m:
                scores.append(float(m.group(1)))
        except Exception:
            pass
    return sum(scores) / len(scores) if scores else None

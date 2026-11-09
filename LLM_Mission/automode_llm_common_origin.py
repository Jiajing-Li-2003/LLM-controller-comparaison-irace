#!/usr/bin/env python3
"""
automode_llm_common.py

Module partage avec gestion avancee des prompts (Natural Language), 
memoire d'erreurs (Self-Reflection) et validation topologique.
"""

"""
automode_llm_common_origin.py

BRAS TEMOIN : reprise a l'identique du module d'origine (LLM_DIR), adapte au
seul dossier LLM_Mission. Conserve volontairement :
  - TASK_DESCRIPTIONS avec les paragraphes qui donnent la strategie au LLM
  - CONDITION_CATALOG documentant '--b<I>x<K>' / '--bi<I>x<K>', qui n'existent
    pas dans AutoMoDe
  - validate_scg_topology, qui exige un graphe fortement connexe de la seule
    branche LLM
  - le prompt sans description de l'arene ni principes generaux de conception
  - EVAL_SEEDS a 3 graines fixes, non decomptees du budget

Ne pas "corriger" ce fichier : sa raison d'etre est de reproduire l'ancien
protocole pour mesurer ce que les corrections ont apporte.
"""

import sys, re, os, subprocess
import anthropic

# Configuration
ANTHROPIC_API_KEY = os.environ.get("ANTHROPIC_API_KEY")

# ---------------------------------------------------------------------------
# Chemins : seule partie adaptee au dossier LLM_Mission.
# Tout le reste du fichier (descriptions de tache, catalogues, prompt,
# conversion CLI, filtre SCG, evaluation) est REPRIS TEL QUEL de la version
# d'origine, y compris ses defauts. C'est le but : ce fichier sert de bras
# temoin, pour mesurer ce que les corrections ont change.
# ---------------------------------------------------------------------------
LLM_DIR = os.path.dirname(os.path.abspath(__file__))
_MARKER = os.path.join("software", "argos3-installation", "automode", "irace-files")


def _find_mandarina_root(start: str):
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
    print("         Corrige avec : export MANDARINA_ROOT=/chemin/vers/le/depot")
    sys.exit(1)

BASE = f"{MANDARINA}/software/argos3-installation/automode"
EXE  = f"{BASE}/bin/automode_main"

# Instances publiees, conservees comme reference
ARGOS = {
    "stop":        f"{BASE}/irace-files/irace-stop/experiments-folder/sequence-1.argos",
    "aggregation": f"{BASE}/irace-files/irace-aggregation/experiments-folder/sequence-1.argos",
    "foraging":    f"{BASE}/irace-files/irace-foraging/experiments-folder/sequence-1.argos",
}
GRAMMAR_FILES = {
    "stop":        f"{BASE}/irace-files/irace-stop/grammars/sq-grammar.txt",
    "aggregation": f"{BASE}/irace-files/irace-aggregation/grammars/sq-grammar.txt",
    "foraging":    f"{BASE}/irace-files/irace-foraging/grammars/sq-grammar.txt",
}

# Le banc d'instances est PARTAGE avec le bras courant : les deux bras doivent
# affronter exactement les memes missions, sinon la comparaison ne vaut rien.
MISSIONS_DIR = os.path.join(LLM_DIR, "missions")
# Les sorties, elles, sont separees pour ne pas ecraser le bras courant.
RESULTS_DIR = os.path.join(LLM_DIR, "resultats_origin")
RUNS_DIR = os.path.join(LLM_DIR, "runs_origin")

TASKS = ["aggregation", "stop", "foraging"]


def mission_dir(task: str, idx: int) -> str:
    return os.path.join(MISSIONS_DIR, task, f"mission-{idx:02d}")


def mission_argos(task: str, idx: int) -> str:
    return os.path.join(mission_dir(task, idx), "sequence-1.argos")


def check_base():
    problems = []
    if not os.path.isfile(EXE):
        problems.append(f"automode_main introuvable : {EXE}")
    for t in TASKS:
        if not os.path.isfile(GRAMMAR_FILES[t]):
            problems.append(f"grammaire manquante : {GRAMMAR_FILES[t]}")
    if problems:
        print("[ERREUR] Le depot de base n'est pas pret :")
        for p in problems:
            print("  - " + p)
        sys.exit(1)


EVAL_SEEDS = [12345, 42, 999]
ERROR_MEMORY_FILE_TEMPLATE = os.path.join(RUNS_DIR, "llm_errors_memory_{task}.txt")

def check_api_key():
    if not ANTHROPIC_API_KEY:
        print("[ERREUR] La variable d'environnement ANTHROPIC_API_KEY n'est pas definie.")
        sys.exit(1)

TASK_DESCRIPTIONS = {
    "stop": (
        "The objective of the 'Stop' task is to minimize the total distance traveled by the swarm of robots. "
        "The optimal physical behavior is usually for all robots to immediately stop moving and remain perfectly still. "
        "The fitness score is calculated based on movement; therefore, a LOWER score is better and indicates less movement."
    ),
    "aggregation": (
        "The objective of the 'Aggregation' task is for the swarm of scattered robots to find and cluster together "
        "on a specific target area (typically a black floor spot) in the arena. The robots must explore to find the spot, "
        "and once found, they should stop and stay there to form a single dense cluster. "
        "The fitness score measures the distance of the robots from each other and the target; "
        "a LOWER score is better and indicates a tighter, more successful aggregation."
    ),
    "foraging": (
        "The objective of the 'Foraging' task is for the swarm of robots to locate food items in a specific area "
        "and transport them back to a designated nest area. The optimal behavior involves establishing an efficient "
        "flow of robots traveling back and forth between the food source and the nest. "
        "The fitness score is calculated to reward the number of items successfully retrieved. "
        "Because the optimizer always MINIMIZES the score, a LOWER (often more negative) score means more items were collected and indicates a better performance."
    )
}

BEHAVIOR_CATALOG = {
    "0":  ("Exploration", "moves and turns randomly", [("RWM0", "--rwm<I>", "turning rate")]),
    "1":  ("Stop", "stands completely still", []),
    "2":  ("Phototaxis", "moves toward the light source", []),
    "3":  ("AntiPhototaxis", "moves away from the light source", []),
    "4":  ("Attraction", "moves toward nearby robots", [("ATT0", "--att<I>", "attraction gain")]),
    "5":  ("Repulsion", "moves away from nearby robots", [("REP0", "--rep<I>", "repulsion gain")]),
    "6":  ("Spiral", "traces an expanding spiral", [("SPD0", "--spd<I>", "spiral speed")]),
    "8":  ("GoToColor", "moves toward a target color", [("VEL0", "--vel<I>", "fixed 1.0"), ("CLR0", "--clr<I>", "color 1-6")]),
    "9":  ("GoAwayColor", "moves away from a color", [("VEL0", "--vel<I>", "fixed 1.0"), ("CLR0", "--clr<I>", "color 1-6")]),
    "10": ("FlockCenter", "moves toward group center", [("FC0", "--fc<I>", "flocking gain")]),
    "11": ("ZigZag", "moves in a zigzag pattern", [("ZFR0", "--zfr<I>", "zigzag frequency")]),
    "12": ("Rotation", "spins in place on the spot (no net translation)", [("ROT0", "--rot<I>", "signed rotation speed, negative = counter-clockwise")]),
    "13": ("WallFollowing", "hugs and follows the nearest wall/obstacle boundary", [("WFS0", "--wfs<I>", "following gain"), ("WFD0", "--wfd<I>", "side: 0=left,1=right")]),
    "14": ("EdgeFollowing", "follows the boundary between two floor colors using the ground sensors", [("GFG0", "--gfg<I>", "correction gain")]),
    "15": ("Orbital", "circles around the light source instead of moving straight to it", [("ORB0", "--orb<I>", "radial attraction gain")]),
}

CONDITION_CATALOG = {
    "0": ("BLACK floor", [("P0x0", "--p<I>x<K>", "probability")]),
    "1": ("GRAY floor", [("P0x0", "--p<I>x<K>", "probability")]),
    "2": ("WHITE floor", [("P0x0", "--p<I>x<K>", "probability")]),
    "3": ("Neighbor count ABOVE threshold", [("B0x0", "--b<I>x<K>", "threshold"), ("W0x0", "--w<I>x<K>", "steepness")]),
    "4": ("Neighbor count BELOW threshold", [("BI0x0", "--bi<I>x<K>", "threshold"), ("WI0x0", "--wi<I>x<K>", "steepness")]),
    "5": ("Fixed probability", [("P0x0", "--p<I>x<K>", "probability")]),
    "6": ("Obstacle detected nearby", [("P0x0", "--p<I>x<K>", "probability"), ("OTH0x0", "--oth<I>x<K>", "proximity threshold 0-1")]),
    "7": ("Sees LED color", [("L0x0", "--l<I>x<K>", "color 1-6"), ("P0x0", "--p<I>x<K>", "probability")]),
    "8": ("Time spent in current state", [("P0x0", "--p<I>x<K>", "probability"), ("TT0x0", "--tt<I>x<K>", "ticks threshold 1-300")]),
    "9": ("Neighbor count WITHIN a range", [("P0x0", "--p<I>x<K>", "probability"), ("NLO0x0", "--nlo<I>x<K>", "range lower bound"), ("NHI0x0", "--nhi<I>x<K>", "range upper bound")]),
}

BEHAVIOR_PARAM_DEFAULTS = {"RWM": "50", "ATT": "3", "REP": "3", "SPD": "5", "FC": "3", "ZFR": "5", "VEL": "1.0", "CLR": "1",
                           "ROT": "50", "WFS": "50", "WFD": "0", "GFG": "50", "ORB": "50"}

SYSTEM_PROMPT_TEMPLATE = """You are an expert in swarm robotics, specifically specializing in the ARGoS3 simulator and the AutoMoDe framework.
Your role is to act as the intelligent core of an evolutionary algorithm. You will translate natural language reasoning and simulation feedback into valid AutoMoDe Finite State Machine (FSM) strings.

# 1. THE PHYSICAL TASK OBJECTIVE
{task_description}

# 2. TASK EXECUTION: TWO-STEP PROCESS
**STEP 1: ANALYSIS (Reasoning)**
Before generating the FSM strings, you must explicitly write out your reasoning in natural language:
- State how many states your machine needs and describe the physical behavior of each state.
- Explain the environmental conditions that will trigger a transition from one state to another.
- Carefully calculate the exact destination index (D) for each transition, strictly applying the "Destination Index Rule" detailed below.

**STEP 2: FSM GENERATION**
Output the final FSM on a single, continuous line starting strictly with the prefix "FSM: ". Do not add any extra formatting to the line itself.

---

# 3. AUTOMODE FSM FORMAT REFERENCE
The FSM is defined entirely as a command-line string.
General pattern: `--nstates N --s<I> B [behavior params] --cle<I> L [--n<I> T --n<I>x<K> D --c<I>x<K> C --p<I>x<K> P [--w<I>x<K> W] [--l<I>x<K> LED]]`
*(Note: `<I>` is the current state index, and `<K>` is the transition index for that state. Both are 0-indexed.)*

## A. Global Parameters
* `--nstates N`: Total number of states (integer, {nstates_domain}).

## B. Behaviors (State Definitions)
Define the behavior for each state using `--s<I> B`.
**ONLY use the following behavior IDs valid for this specific task:**
{behaviors_doc}

All behaviors strictly require an LED color parameter:
* `--cle<I> L`: LED color while in this state. Valid values: {cle_domain} (0=Off, 1=Red, 2=Green, 3=Blue, 4=Yellow, 5=Magenta, 6=Cyan).

## C. Transitions
Define the transitions originating from each state `<I>`:
* `--n<I> T`: Total number of outgoing transitions from state `<I>` ({max_transitions_domain}). Use 0 if there are none.
  **Do not default to a single simple transition out of habit.** The search space allows up to 4 transitions per state, each with a *different* condition type (floor color, neighbor count, obstacle, time-in-state, sees-LED, etc.). A state with several transitions covering different situations is a legitimate and often stronger strategy than a state with one transition — irace's random sampling naturally produces this kind of richer branching, so do not be more conservative than a random search would be.
* `--n<I>x<K> D`: Destination index of the TARGET state. 
  **CRITICAL DESTINATION INDEX RULE:** D is the index of the target state among all *OTHER* states (0-indexed, excluding the current state).
  * *Example (3 states: 0, 1, 2):* If transitioning FROM state 1, the other available states are [0, 2]. To target state 0, D=0. To target state 2, D=1.
  * *Example (2 states: 0, 1):* If transitioning FROM state 0, the only other state is [1]. Therefore, D must be 0.
* `--c<I>x<K> C`: Condition type. 
  **ONLY use the following condition IDs valid for this task:**
{conditions_doc}

---

# 4. STRICT CONSTRAINTS
1. **Type Safety:** Integers must be plain integers (0, 1, 2). Real-valued parameters must always include a decimal point (e.g., 1.0, 0.5).
2. **Minimum Topology (MANDATORY):** `--nstates` must always be at least 2. EVERY state, without exception, must have at least one outgoing transition (`--n<I>` >= 1).
3. **Escape State 0:** State 0 (S0) is the initial state. It MUST have at least one transition pointing to ANOTHER state.
4. **No Self-Loops:** Never write a transition pointing to the robot's own current state. Transitions must always connect to a DIFFERENT state.
5. **Completeness:** You MUST include ALL required parameters for every behavior and condition used. Missing parameters will crash the simulator.
6. **STRONGLY CONNECTED GRAPH (MANDATORY TOPOLOGY):** Your FSM MUST form a Strongly Connected Graph (SCG). There must be a valid directed path from ANY state to EVERY OTHER state in the FSM. The robot must never get permanently trapped in a single state. For 2 states: S0 MUST have at least one transition pointing to S1, AND S1 MUST have at least one transition pointing to S0. For 3 states: You must form at least a complete directed cycle (e.g., S0 to S1, S1 to S2, S2 to S0) OR ensure bidirectional connections between all states. Self-loops do NOT count: A transition pointing to its own state does not help connectivity. You MUST ensure transitions reach other states.
"""

def load_error_memory(task: str) -> list:
    fpath = ERROR_MEMORY_FILE_TEMPLATE.format(task=task)
    if os.path.exists(fpath):
        with open(fpath, "r") as f:
            return [line.strip() for line in f.readlines() if line.strip()]
    return []

def save_error_memory(task: str, errors: list):
    os.makedirs(RUNS_DIR, exist_ok=True)
    unique_errors = list(dict.fromkeys(errors))
    unique_errors = unique_errors[-20:]
    fpath = ERROR_MEMORY_FILE_TEMPLATE.format(task=task)
    with open(fpath, "w") as f:
        for e in unique_errors:
            f.write(e + "\n")

def _split_domain(domain_str: str) -> list:
    parts, current, in_quotes = [], '', False
    for ch in domain_str:
        if ch == '"':
            in_quotes = not in_quotes
            current += ch
        elif ch == ',' and not in_quotes:
            parts.append(current)
            current = ''
        else:
            current += ch
    parts.append(current)
    return [p.strip() for p in parts]

def parse_grammar(grammar_file: str) -> list:
    params = []
    try:
        with open(grammar_file) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'): continue
                if '|' in line:
                    main_part, cond_part = line.split('|', 1)
                    cond_part = cond_part.strip()
                else:
                    main_part, cond_part = line, None
                m = re.match(r'^(\S+)\s+"([^"]*)"\s+([icr])\s*\(([^)]*)\)', main_part.strip())
                if not m: continue
                name, switch, ptype, domain_str = m.groups()
                if ptype == 'c':
                    domain = [v.strip() for v in domain_str.split(',')]
                else:
                    lo_s, hi_s = _split_domain(domain_str)
                    try:
                        lo = float(lo_s)
                    except ValueError:
                        lo = 0.0
                    try:
                        hi = float(hi_s)
                    except ValueError:
                        hi = 3.0
                    domain = (lo, hi)
                params.append({"name": name, "switch": switch.strip(), "type": ptype, "domain": domain, "condition": cond_part})
    except FileNotFoundError:
        print(f"[ERREUR] Fichier de grammaire introuvable a {grammar_file}")
        sys.exit(1)
    return params

def describe_param(meta_key: str, param_meta: dict) -> str:
    meta = param_meta.get(meta_key)
    if not meta: return "?"
    if meta["type"] == "c":
        if len(meta["domain"]) == 1: return f"fixed value {meta['domain'][0]}"
        return "choice of {" + ",".join(meta["domain"]) + "}"
    lo, hi = meta["domain"]
    lo_s = str(int(lo)) if lo == int(lo) else str(lo)
    hi_s = str(int(hi)) if hi == int(hi) else str(hi)
    kind = "integer" if meta["type"] == "i" else "real number"
    return f"{kind} [{lo_s}..{hi_s}]"

def build_behaviors_doc(param_meta: dict) -> str:
    s0 = param_meta.get("S0")
    valid_ids = sorted(s0["domain"], key=lambda x: int(x))
    lines = []
    for bid in valid_ids:
        if bid not in BEHAVIOR_CATALOG: continue
        label, desc, reqs = BEHAVIOR_CATALOG[bid]
        line = f"  B={bid}  {label:<14}→ {desc}"
        for meta_key, switch_pat, note in reqs:
            line += f"\n                        requires: {switch_pat}  {describe_param(meta_key, param_meta)}  ({note})"
        lines.append(line)
    return "\n".join(lines)

def build_conditions_doc(param_meta: dict) -> str:
    c0 = param_meta.get("C0x0")
    valid_ids = sorted(c0["domain"], key=lambda x: int(x))
    lines = []
    for cid in valid_ids:
        if cid not in CONDITION_CATALOG: continue
        label, reqs = CONDITION_CATALOG[cid]
        line = f"  {cid} = {label}"
        for meta_key, switch_pat, note in reqs:
            line += f"\n                  requires {switch_pat}  {describe_param(meta_key, param_meta)}  ({note})"
        lines.append(line)
    return "\n".join(lines)

def load_grammar(task: str):
    grammar_path = GRAMMAR_FILES.get(task)
    grammar_params = parse_grammar(grammar_path)
    irace_params = [p["name"] for p in grammar_params]
    param_meta = {p["name"]: p for p in grammar_params}
    return grammar_params, irace_params, param_meta

def build_prompt_context(task: str, param_meta: dict):
    task_desc = TASK_DESCRIPTIONS.get(task, f"Optimize the '{task}' behavior for the swarm.")
    system_prompt = SYSTEM_PROMPT_TEMPLATE.format(
        task_description=task_desc,
        nstates_domain=describe_param("NumStates", param_meta),
        max_transitions_domain=describe_param("NumConnections0", param_meta),
        behaviors_doc=build_behaviors_doc(param_meta),
        cle_domain=describe_param("CLE0", param_meta),
        conditions_doc=build_conditions_doc(param_meta),
    )
    valid_behavior_ids = sorted(param_meta["S0"]["domain"], key=lambda x: int(x))
    valid_behavior_names = [BEHAVIOR_CATALOG[b][0] for b in valid_behavior_ids if b in BEHAVIOR_CATALOG]
    return system_prompt, valid_behavior_names, ""

def cli_to_irace_dict(cli: str, task: str, param_meta: dict, irace_params: list):
    errors_found = []
    tokens = cli.split()
    raw = {}
    i = 0
    while i < len(tokens):
        if tokens[i].startswith("--") and i + 1 < len(tokens) and not tokens[i + 1].startswith("--"):
            raw[tokens[i][2:]] = tokens[i + 1]
            i += 2
        else: i += 1

    d = {p: "NA" for p in irace_params}
    nstates_str = raw.get("nstates", "NA")
    try:
        nstates = int(nstates_str) if nstates_str != "NA" else 0
        if nstates == 0:
            nstates = 1
            errors_found.append("Missing '--nstates'. AutoMoDe crashed. You MUST declare '--nstates'.")
    except ValueError:
        nstates = 1
        errors_found.append(f"Invalid '--nstates {nstates_str}'. Must be an integer.")
    d["NumStates"] = str(nstates)

    for s in range(min(nstates, 4)):
        for prefix in ["s", "rwm", "att", "rep", "vel", "cle", "clr", "spd", "fc", "zfr", "rot", "wfs", "wfd", "gfg", "orb"]:
            if raw.get(f"{prefix}{s}") is not None:
                d[f"{prefix.upper()}{s}"] = raw[f"{prefix}{s}"]

        if nstates > 1:
            nc_raw = raw.get(f"n{s}")
            if nc_raw is None:
                nc_val = 1
                errors_found.append(f"State {s} missing '--n{s}' parameter! Every active state MUST declare its number of transitions to prevent a crash.")
            else:
                try: nc_val = int(nc_raw)
                except ValueError: nc_val = 1
                
            if nc_val <= 0:
                d[f"NumConnections{s}"] = "1"
                d[f"N{s}x0"] = "0" if s > 0 else "1"
                d[f"C{s}x0"], d[f"P{s}x0"] = "0", "0.0"
            else:
                nc_val = min(nc_val, 4)
                d[f"NumConnections{s}"] = str(nc_val)
                for k in range(nc_val):
                    xk = f"{s}x{k}"
                    dest = raw.get(f"n{xk}")
                    if dest is None:
                        errors_found.append(f"Transition missing destination '--n{xk}'. You MUST specify the target state index.")
                        dest = "0" if s > 0 else "1"
                    cond = raw.get(f"c{xk}", "0")
                    prob = raw.get(f"p{xk}", "1.0")
                    w, l = raw.get(f"w{xk}", "5.0"), raw.get(f"l{xk}", "1")
                    oth = raw.get(f"oth{xk}", "0.1")
                    tt = raw.get(f"tt{xk}", "10")
                    nlo, nhi = raw.get(f"nlo{xk}", "0"), raw.get(f"nhi{xk}", "5")
                    try: dest_val = int(dest)
                    except ValueError: dest_val = 0
                    d[f"N{s}x{k}"] = str(min(dest_val, max(0, nstates - 2)))
                    d[f"C{s}x{k}"] = cond
                    try: c = int(cond)
                    except ValueError: c = 0
                    if c == 3: d[f"B{s}x{k}"], d[f"W{s}x{k}"] = prob, w
                    elif c == 4: d[f"BI{s}x{k}"], d[f"WI{s}x{k}"] = prob, w
                    elif c == 7: d[f"P{s}x{k}"], d[f"L{s}x{k}"] = prob, l
                    elif c == 6: d[f"P{s}x{k}"], d[f"OTH{s}x{k}"] = prob, oth
                    elif c == 8: d[f"P{s}x{k}"], d[f"TT{s}x{k}"] = prob, tt
                    elif c == 9: d[f"P{s}x{k}"], d[f"NLO{s}x{k}"], d[f"NHI{s}x{k}"] = prob, nlo, nhi
                    else: d[f"P{s}x{k}"] = prob

    for k, v in list(d.items()):
        if v == "NA": continue
        meta = param_meta.get(k)
        if not meta: continue
        if meta["type"] in ("i", "r"):
            lo, hi = meta["domain"]
            try: fv = float(v)
            except ValueError: continue
            if fv < lo or fv > hi:
                clamped = max(lo, min(hi, fv))
                errors_found.append(f"Parameter '--{k.lower()}' was generated as {v}, but the allowed grammar range is [{lo}, {hi}]. Follow the limits.")
                d[k] = str(int(round(clamped))) if meta["type"] == "i" else f"{clamped:.4f}"
        elif meta["type"] == "c":
            if v not in meta["domain"]:
                errors_found.append(f"Parameter '--{k.lower()}' used invalid choice '{v}'. Must be one of: {meta['domain']}.")
                d[k] = meta["domain"][0]

    for i in range(4):
        s_val = d.get(f"S{i}")
        if s_val is None or s_val == "NA": continue
        entry = BEHAVIOR_CATALOG.get(s_val)
        if not entry:
            errors_found.append(f"Invalid behavior ID '--s{i} {s_val}'. You MUST only use behavior IDs defined in the instructions.")
            d[f"S{i}"] = "0"
            entry = BEHAVIOR_CATALOG.get("0")
        _, _, reqs = entry
        for meta_key, _switch, _note in reqs:
            prefix = re.sub(r'\d+$', '', meta_key)
            target_key = f"{prefix}{i}"
            if d.get(target_key, "NA") == "NA":
                errors_found.append(f"Behavior '--s{i} {d[f'S{i}']}' was used but the REQUIRED parameter '--{target_key.lower()}' was missing. This crashes AutoMoDe.")
                d[target_key] = BEHAVIOR_PARAM_DEFAULTS.get(prefix, "1")
        if d.get(f"CLE{i}", "NA") == "NA":
            errors_found.append(f"State {i} is missing the required '--cle{i}' parameter for LED color.")
            d[f"CLE{i}"] = "0"

    return d, errors_found

def dict_to_cli(d: dict, param_meta: dict, irace_params: list) -> str:
    tokens = [f"--nstates {d.get('NumStates', '1')}"]
    for p in irace_params:
        val = d.get(p, "NA")
        if p != "NumStates" and val != "NA" and p in param_meta:
            tokens.append(f"{param_meta[p]['switch']} {val}")
    return " ".join(tokens)

def write_configurations_file(path: str, irace_params: list, dict_rows: list):
    with open(path, "w") as f:
        f.write("\t".join(irace_params) + "\n")
        for d in dict_rows:
            f.write("\t".join(d[p] for p in irace_params) + "\n")

def fix_transition_destinations(cli: str) -> str:
    tokens = cli.split()
    nstates = 1
    for i, t in enumerate(tokens):
        if t == "--nstates" and i + 1 < len(tokens):
            try: nstates = int(tokens[i + 1])
            except ValueError: pass
            break
    max_dest = max(0, nstates - 2)
    out = []
    i = 0
    while i < len(tokens):
        tok = tokens[i]
        out.append(tok)
        if re.match(r'^--n\d+x\d+$', tok) and i + 1 < len(tokens):
            i += 1
            try: v = int(float(tokens[i]))
            except ValueError: v = 0
            out.append(str(min(v, max_dest)))
        i += 1
    return " ".join(out)

def validate_scg_topology(d: dict) -> tuple[bool, str]:
    """
    Vérifie si la FSM forme un graphe fortement connexe (Strongly Connected Graph).
    Prend en entrée le dictionnaire des paramètres généré par cli_to_irace_dict.
    """
    try: 
        nstates = int(d.get("NumStates", "1") or "1")
    except (ValueError, TypeError): 
        nstates = 1
        
    if nstates < 2: 
        return False, "La FSM doit avoir au moins 2 etats."

    graph = {s: set() for s in range(nstates)}
    
    for s in range(nstates):
        try: 
            nconn = int(d.get(f"NumConnections{s}", "0") or "0")
        except ValueError: 
            nconn = 0
            
        if nconn == 0:
            return False, f"L'etat S{s} est une impasse (0 transition sortante)."
            
        others = [x for x in range(nstates) if x != s]
        
        for k in range(nconn):
            dest_raw = d.get(f"N{s}x{k}", "0")
            try: 
                dest_idx = int(dest_raw)
            except ValueError: 
                dest_idx = 0
            
            # Application exacte du recadrage de destination d'AutoMoDe
            dest_idx_clamped = min(dest_idx, max(0, nstates - 2))
            if others:
                graph[s].add(others[dest_idx_clamped])

    # Parcours en largeur (BFS)
    def reachable_from(start, g):
        visited = set()
        queue = [start]
        while queue:
            node = queue.pop(0)
            if node not in visited:
                visited.add(node)
                queue.extend(g[node] - visited)
        return len(visited) == nstates

    # Vérification que TOUS les états peuvent atteindre TOUS les autres
    for state in range(nstates):
        if not reachable_from(state, graph):
            return False, f"Le graphe n'est pas fortement connexe. Piege ou inaccessibilite depuis S{state}."

    return True, "Valide"

def evaluate_fsm(cli: str, task: str, argos_file: str = None) -> float | None:
    # Seule adaptation : l'instance peut etre celle de la mission courante au
    # lieu de l'unique sequence-1.argos publiee. Le reste (3 graines fixes,
    # timeout, extraction du score) est inchange.
    cli = fix_transition_destinations(cli)
    env = {**os.environ, "QT_QPA_PLATFORM": "offscreen", "DISPLAY": ""}
    scores = []
    for seed in EVAL_SEEDS:
        cmd = [EXE, "-n", "-c", argos_file or ARGOS[task], "--seed", str(seed),
               "--fsm-config"] + cli.split()
        try:
            r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=60, env=env)
            m = re.search(r'Score\s+([-+]?\d+(?:\.\d+)?)', r.stdout)
            if m: scores.append(float(m.group(1)))
        except Exception: pass
    return sum(scores) / len(scores) if scores else None
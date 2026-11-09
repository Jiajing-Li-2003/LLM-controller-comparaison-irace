#!/usr/bin/env python3
"""
run_paired_comparaison_complet.py

Meme protocole apparie que run_paired_comparaison_complet.py — etape 0,
branche A irace, branche B LLM sur 10 rounds — avec deux changements :

  1. PAS DE BANC. On enchaine 10 repetitions sur l'instance PUBLIEE de la
     tache, sans faire varier ses parametres. Les repetitions ne different
     que par la graine.

  2. RETOUR AU LLM : au lieu des 5 meilleures et des 3 pires configurations,
     le message de round donne le CLASSEMENT COMPLET du round, de la meilleure
     a la pire, et explique la regle generale — reprendre les caracteristiques
     des machines les mieux classees, eviter celles des dernieres. C'est au
     modele de decider ou passe la frontiere.

Sorties dans resultats_complet/ et runs_complet/.

Usage :
    python3 run_paired_comparaison_complet.py aggregation
    python3 run_paired_comparaison_complet.py stop --repetitions 1-5
    python3 run_paired_comparaison_complet.py aggregation --status

Les rapports gardent le nom PAIRED_COMPARISON_<tache>_mission-NN.txt pour
rester lisibles par plot_comparaison.py et export_resultats.py ; ici "NN"
designe la repetition NN, pas une mission d'un banc.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import time

import anthropic

from automode_complet import (
    check_api_key, check_base, load_grammar, build_prompt_context,
    ANTHROPIC_API_KEY, MODEL, TEMPERATURE,
    cli_to_irace_dict, dict_to_cli, evaluate_fsm, write_configurations_file,
    extract_all_fsm_lines, validate_topology,
    load_error_memory, save_error_memory, clear_error_memory,
    BASE, LLM_DIR, RESULTS_DIR, RUNS_DIR, reference_argos, published_params, TASKS, target_runner, grammar_file,
)

# ===========================================================================
# PARAMETRES DE LA CAMPAGNE
# ===========================================================================
N_REPETITIONS = 10      # repetitions sur l'instance publiee
CRN_SEED = 20260914     # base des graines communes, une par repetition
N_ROUNDS = 10
BUDGET_TOTAL = 6000
BUDGET_PER_ROUND = BUDGET_TOTAL // N_ROUNDS
FIRST_TEST = 40
MU = 5

MAX_LLM_BATCH_PER_ROUND = 20
MIN_LLM_BATCH_PER_ROUND = 3
MAX_RETRIES = 3

# Pre-filtrage des FSM du LLM par simulation sur la mission courante.
# Les simulations utilisees sont DECOMPTEES du budget de la branche B, sinon
# elle disposerait d'un budget cache (jusqu'a +25 % dans la version d'origine).
PRE_SCREEN = True
PRE_SCREEN_SEEDS = [12345, 42, 999]

# Nombres aleatoires communs (common random numbers).
# Sans graine fixee, chaque appel a irace tire sa propre graine : l'etape 0, la
# branche A et le round 1 de la branche B mesurent le MEME pool de 14 FSM sur
# des graines ARGoS differentes, et trouvent des scores differents (ecart-type
# ~2.6 % sur la campagne aggregation). Avec cette option, ces trois mesures
# partagent la graine de la mission : le point de depart devient commun et la
# difference A-B est moins bruitee.
COMMON_RANDOM_NUMBERS = True


def mission_seed(task: str, idx: int) -> int:
    """Graine irace deterministe et propre a chaque (tache, mission)."""
    return (CRN_SEED * 1000 + sum(ord(c) for c in task) * 100 + idx) % 2_000_000_000


# Seuil au-dela duquel une FSM est jugee inexploitable. Calcule par tache :
# le pire score possible vaut nombre_de_robots x nombre_de_pas pour les taches
# a penalite unitaire. None = pas de seuil (foraging, dont le score est negatif).
WORST_UNIT_PENALTY = {"aggregation": 1.0, "stop": 1.0, "foraging": None}
TICKS_PER_RUN = 1200


# ===========================================================================
# OUTILS IRACE
# ===========================================================================
def get_irace_bin() -> str:
    r = subprocess.run(["Rscript", "-e",
                        'cat(system.file("bin", "irace", package="irace"))'],
                       capture_output=True, text=True)
    p = r.stdout.strip()
    if not p or not os.path.isfile(p):
        print("[ERREUR] irace introuvable. Dans R : install.packages(\"irace\")")
        sys.exit(1)
    return p


def write_scenario(path, task, exec_dir, instances_dir, budget,
                   nb_iterations=None, config_file=None, seed=None):
    os.makedirs(exec_dir, exist_ok=True)
    lines = [
        f'parameterFile     = "{grammar_file(task)}"',
        f'execDir           = "{exec_dir}"',
        f'trainInstancesDir = "{instances_dir}"',
        f'targetRunner      = "{target_runner(task)}"',
        f'maxExperiments    = {budget}',
        f'firstTest         = {FIRST_TEST}',
        f'mu                = {MU}',
    ]
    if nb_iterations is not None:
        lines.append(f'nbIterations      = {nb_iterations}')
    if config_file:
        lines.append(f'configurationsFile = "{config_file}"')
    if seed is not None:
        lines.append(f'seed              = {seed}')
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")


def run_irace_round(irace_bin, scenario_path, rdata_path, log_path) -> bool:
    os.makedirs(os.path.dirname(log_path), exist_ok=True)
    with open(log_path, "w") as logf:
        proc = subprocess.Popen([irace_bin, "--scenario", scenario_path,
                                 "--log-file", rdata_path],
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, bufsize=1)
        for line in proc.stdout:
            print(line, end="", flush=True)
            logf.write(line)
        proc.wait()
    return proc.returncode == 0 and os.path.isfile(rdata_path)


def extract_round_results(rdata_path, tsv_path) -> bool:
    r_script = os.path.join(LLM_DIR, "extract_iteration_results.R")
    proc = subprocess.run(["Rscript", r_script, rdata_path, tsv_path],
                          capture_output=True, text=True)
    if proc.returncode != 0:
        print(proc.stderr[:600], flush=True)
    return os.path.isfile(tsv_path)


def read_results_tsv(tsv_path, meta, irace_params) -> list:
    out = []
    with open(tsv_path) as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            if not line.strip():
                continue
            row = dict(zip(header, line.rstrip("\n").split("\t")))
            try:
                score = float(row.get("MeanCost", "NA"))
            except ValueError:
                continue
            d = {p: row.get(p, "NA") for p in irace_params}
            out.append((score, dict_to_cli(d, meta, irace_params), d,
                        int(row.get("NEval", "0") or 0)))
    out.sort(key=lambda x: x[0])
    return out


def parse_nb_configurations(log_path) -> int:
    if not os.path.exists(log_path):
        return MAX_LLM_BATCH_PER_ROUND
    m = re.findall(r"#\s*nbConfigurations:\s*(\d+)", open(log_path).read())
    return int(m[0]) if m else MAX_LLM_BATCH_PER_ROUND


def parse_irace_iterations(log_path) -> list:
    """Trace iteration par iteration de la branche A, lue dans le log d'irace.

    irace imprime a chaque iteration un en-tete (# Iteration N of M,
    # experimentsUsed, # nbConfigurations) puis, une fois la course finie,
    une ligne 'Best-so-far configuration: <ID> mean value: <score>'.
    On rassemble les deux pour obtenir l'equivalent de l'historique que la
    branche B produit deja round par round.
    """
    if not os.path.isfile(log_path):
        return []
    text = open(log_path, errors="ignore").read()

    # la ligne reelle est "# 2026-09-16 17:19:42 EDT: Iteration 1 of 10",
    # donc on ne peut pas ancrer sur "#" suivi de "Iteration".
    heads = list(re.finditer(r"Iteration\s+(\d+)\s+of\s+(\d+)", text))
    if not heads:
        return []

    rows = []
    for i, h in enumerate(heads):
        block = text[h.end(): heads[i + 1].start() if i + 1 < len(heads) else len(text)]

        def grab(pattern, default=None, cast=int):
            m = re.search(pattern, block)
            if not m:
                return default
            try:
                return cast(m.group(1))
            except ValueError:
                return default

        used = grab(r"#\s*experimentsUsed:\s*(\d+)", 0)
        nbconf = grab(r"#\s*nbConfigurations:\s*(\d+)", 0)
        budget = grab(r"#\s*currentBudget:\s*(\d+)", 0)

        best = re.findall(
            r"Best-so-far configuration:\s*(\d+)\s*mean value:\s*([-\d.eE+]+)", block)
        cfg_id, score = (best[-1][0], float(best[-1][1])) if best else (None, None)

        rows.append(dict(iteration=int(h.group(1)), total=int(h.group(2)),
                         used_before=used, budget=budget, nbconf=nbconf,
                         cfg_id=cfg_id, best=score))

    # evaluations cumulees a la FIN de chaque iteration
    for i, r in enumerate(rows):
        r["used_after"] = rows[i + 1]["used_before"] if i + 1 < len(rows) \
            else r["used_before"] + r["budget"]
    return rows


def format_history_a(rows: list) -> str:
    if not rows:
        return "  (log d'irace illisible ou absent)\n"
    out = ["  iter | best-so-far |  config |  nbConf | evaluations cumulees",
           "  -----+-------------+---------+---------+---------------------"]
    for r in rows:
        best = f"{r['best']:11.2f}" if r["best"] is not None else "          ?"
        out.append(f"  {r['iteration']:4d} | {best} | {str(r['cfg_id'] or '?'):>7s} "
                   f"| {r['nbconf']:7d} | {r['used_after']:19d}")
    return "\n".join(out) + "\n"


def branche_a_terminee(log_path) -> bool:
    """Vrai seulement si irace est alle au bout de ses N_ROUNDS iterations.

    L'ancien test se contentait de trouver une ligne "Best-so-far" : une course
    interrompue au bout de 2 iterations passait pour finie, et la reprise
    gardait un score obtenu avec un cinquieme du budget.
    """
    rows = parse_irace_iterations(log_path)
    if not rows:
        return False
    last = rows[-1]
    if last["iteration"] != last["total"] or last["best"] is None:
        return False
    return "Best configurations as commandlines" in open(log_path, errors="ignore").read()


def config_key(d: dict, params: list) -> tuple:
    """Signature d'une configuration, independante du formatage des nombres.

    Comparer les chaines CLI ne marche pas : le pool retient ce que le LLM a
    ecrit ("0.9"), tandis que la configuration relue du TSV passe par R et
    revient parfois autrement formatee. Une FSM du LLM etait alors etiquetee
    "issue d'irace" a tort.
    """
    out = []
    for c in params:
        v = d.get(c, "NA")
        try:
            out.append(f"{float(v):.4f}")
        except (TypeError, ValueError):
            out.append(str(v))
    return tuple(out)


def try_extract_irace_best(log_path):
    if not os.path.isfile(log_path):
        return None, None
    text = open(log_path).read()
    sc = re.findall(r"Best-so-far configuration:\s*\d+\s*mean value:\s*([\d.eE+-]+)", text)
    score = float(sc[-1]) if sc else None
    cli, capture = None, False
    for line in text.split("\n"):
        if "# Best configurations as commandlines" in line:
            capture = True
            continue
        if capture and "--nstates" in line and not line.startswith("#"):
            cli = " ".join(line.split()[1:])
            break
    return score, cli


# ===========================================================================
# MESSAGES AU LLM
# ===========================================================================
def fmt_entry(e) -> str:
    return f"Score: {e[0]:.2f} -> FSM: {e[1]}"


def build_round_message(task, round_no, budget_r, ranked_round, best_ever, target_next,
                        work_dir=None):
    """Message de retour au LLM, avec le CLASSEMENT COMPLET du round.

    Difference avec la version a 5 meilleures / 3 pires : on ne choisit pas a
    la place du modele ou se situe la frontiere entre ce qu'il faut reprendre
    et ce qu'il faut eviter. On lui donne tout le classement et la regle
    generale, il tranche lui-meme.
    """
    n = len(ranked_round)
    largeur = len(str(n))
    classement = "\n".join(
        f"  {i+1:>{largeur}}/{n}. {fmt_entry(e)}" for i, e in enumerate(ranked_round)
    ) or "  (none)"
    champ = f"Score: {best_ever[0]:.2f} -> FSM: {best_ever[1]}" if best_ever else "(none yet)"

    msg = (
        f"Round {round_no} out of {N_ROUNDS} has just completed on the {task.upper()} mission.\n"
        f"As a reminder, the optimiser MINIMISES the score: lower is always better.\n"
        f"The next round has a budget of {budget_r} simulations.\n\n"
        f"=== CURRENT BEST EVER (THE CHAMPION) ===\n"
        f"You MUST include an exact copy of this machine in your next answer so it survives:\n"
        f"  {champ}\n\n"
        f"=== FULL RANKING OF THIS ROUND ===\n"
        f"All {n} machines evaluated in round {round_no}, ranked from best to worst:\n"
        f"{classement}\n\n"
        f"As a general rule, the machines at the top of this ranking carry the "
        f"characteristics worth keeping, and those at the bottom carry the "
        f"characteristics worth avoiding. Where exactly the boundary lies is for "
        f"you to judge from the scores: a large gap between two neighbouring "
        f"entries is more informative than their rank.\n\n"
        f"=== YOUR TASK FOR ROUND {round_no + 1} ===\n"
        f"Supply exactly {target_next} machines to form the next population.\n"
        f"Write your reasoning first, then each machine on its own line starting with 'FSM: '.\n"
    )
    errs = load_error_memory(task, work_dir)
    if errs:
        msg += "\n[ERRORS DETECTED IN YOUR PREVIOUS ANSWERS — do not repeat them]\n"
        msg += "".join(f"- {e}\n" for e in errs[-8:])
    return msg


def call_llm(client, system_prompt, messages, target_next, api_log):
    for attempt in range(3):
        try:
            resp = client.messages.create(
                model=MODEL, max_tokens=450 * target_next + 1000,
                temperature=TEMPERATURE, system=system_prompt, messages=messages)
            if resp.stop_reason == "max_tokens":
                print("   [DIAG] Reponse tronquee (max_tokens) : des lignes 'FSM: ' "
                      "manquent peut-etre a la fin.", flush=True)
            return resp.content[0].text, None
        except Exception as e:
            print(f"   [DIAG] Appel API en echec ({attempt+1}/3) : "
                  f"{type(e).__name__}: {e}", flush=True)
            with open(api_log, "a") as f:
                f.write(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] "
                        f"{type(e).__name__}: {e}\n")
            if attempt < 2:
                time.sleep(5 * (2 ** attempt))
            else:
                return "", e
    return "", None


# ===========================================================================
# JOURNAUX COMPLETS, UN PAR BRANCHE ET PAR MISSION
# ===========================================================================
def llm_proposals(messages: list) -> dict:
    """Associe chaque reponse du LLM au round qu'elle alimente.

    Le message utilisateur "Round N out of ..." donne le retour du round N ;
    la reponse qui suit constitue la population du round N+1. Une relance
    ("Your last answer yielded only ...") vise le meme round : ses reponses
    s'ajoutent a la liste, comme essais successifs.
    """
    out, target = {}, None
    for m in messages:
        if m.get("role") == "user":
            mm = re.match(r"Round (\d+) out of", m.get("content", ""))
            if mm:
                target = int(mm.group(1)) + 1
        elif m.get("role") == "assistant" and target is not None:
            out.setdefault(target, []).append(m.get("content", ""))
    return out


def export_mission_logs(task: str, idx: int, params: dict = None) -> list:
    """Ecrit dans resultats/ un journal complet par branche.

        <TACHE>_mission-NN_irace.txt : les 10 iterations d'irace (branche A)
        <TACHE>_mission-NN_LLM.txt   : les 10 rounds de la branche B, chacun
                                       precede de ce que le LLM a propose
    """
    name = f"mission-{idx:02d}"   # nom de fichier conserve ; il designe une repetition
    work = os.path.join(RUNS_DIR, task, name)
    if not os.path.isdir(work):
        return []
    os.makedirs(RESULTS_DIR, exist_ok=True)
    written = []

    def banner(txt):
        return "#" * 80 + f"\n# {txt}\n" + "#" * 80 + "\n"

    # ---------------------------------------------------------- branche A
    log_a = os.path.join(work, "irace_A.log")
    if os.path.isfile(log_a):
        out_a = os.path.join(RESULTS_DIR, f"{task.upper()}_{name}_irace.txt")
        rows = parse_irace_iterations(log_a)
        with open(out_a, "w") as f:
            f.write(f"JOURNAL DE LA BRANCHE A (irace pur) — {task.upper()} / {name}\n")
            f.write("=" * 80 + "\n")
            f.write(f"Budget {BUDGET_TOTAL} | firstTest {FIRST_TEST} | "
                    f"{N_ROUNDS} iterations | log integral d'irace\n\n")
            f.write("RESUME PAR ITERATION\n" + "-" * 80 + "\n")
            f.write(format_history_a(rows))
            f.write("\n" + banner("LOG INTEGRAL") + "\n")
            f.write(open(log_a, errors="ignore").read())
        written.append(out_a)

    # ---------------------------------------------------------- branche B
    logs_b = {}
    for fn in os.listdir(work):
        m = re.fullmatch(r"irace_B(\d+)\.log", fn)
        if m:
            logs_b[int(m.group(1))] = os.path.join(work, fn)
    if logs_b:
        props = {}
        state_file = os.path.join(work, "etat_B.json")
        if os.path.isfile(state_file):
            try:
                props = llm_proposals(json.load(open(state_file)).get("messages", []))
            except Exception:
                props = {}

        out_b = os.path.join(RESULTS_DIR, f"{task.upper()}_{name}_LLM.txt")
        with open(out_b, "w") as f:
            f.write(f"JOURNAL DE LA BRANCHE B (LGEA LLM) — {task.upper()} / {name}\n")
            f.write("=" * 80 + "\n")
            f.write(f"Budget {BUDGET_TOTAL} | firstTest {FIRST_TEST} | "
                    f"{len(logs_b)} rounds | modele {MODEL}\n")
            f.write("Chaque round : ce que le LLM a propose, puis le log irace "
                    "qui l'a evalue.\n")
            f.write("Le prompt systeme est reproduit en fin de fichier.\n\n")

            for r in sorted(logs_b):
                f.write("\n" + banner(f"ROUND {r}/{len(logs_b)}"))
                f.write("\n--- POPULATION PROPOSEE POUR CE ROUND ---\n")
                if r == 1:
                    f.write("(round 1 : population aleatoire issue de l'etape 0, "
                            "le LLM n'intervient pas encore)\n")
                elif r in props:
                    for k, txt in enumerate(props[r], 1):
                        if len(props[r]) > 1:
                            f.write(f"\n[essai {k}/{len(props[r])}]\n")
                        f.write(txt.rstrip() + "\n")
                else:
                    f.write("(reponse du LLM non retrouvee dans etat_B.json)\n")
                f.write("\n--- LOG IRACE DE CE ROUND ---\n")
                f.write(open(logs_b[r], errors="ignore").read())

            sp = os.path.join(work, "system_prompt.txt")
            if os.path.isfile(sp):
                f.write("\n" + banner("PROMPT SYSTEME") + "\n")
                f.write(open(sp, errors="ignore").read())
        written.append(out_b)

    return written


# ===========================================================================
# REPRISE APRES INTERRUPTION
# ===========================================================================
def etape0_done(seed_file: str) -> bool:
    """Le pool de depart existe-t-il deja et contient-il au moins une FSM ?"""
    if not os.path.isfile(seed_file):
        return False
    with open(seed_file) as f:
        return sum(1 for _ in f) > 1          # en-tete + au moins une ligne


def save_state_b(path, next_round, messages, pool_file, remaining_budget,
                 best_ever, llm_pool_keys, history):
    """Sauve l'etat de la branche B, conversation LLM comprise.

    Sans ca, un Ctrl+C au round 8 ferait tout recommencer au round 1 : le
    budget deja depense ET l'historique de dialogue seraient perdus.
    """
    try:
        with open(path, "w") as f:
            json.dump(dict(next_round=next_round, messages=messages,
                           pool_file=pool_file, remaining_budget=remaining_budget,
                           best_ever=list(best_ever) if best_ever else None,
                           llm_pool_keys=[list(k) for k in llm_pool_keys],
                           history=history), f)
    except Exception as e:
        print(f"   [DIAG] etat B non sauvegarde ({e}) — la reprise repartira du round 1.")


def mission_status(task: str, idx: int) -> dict:
    """Etat d'avancement d'une mission, lu sur le disque."""
    name = f"mission-{idx:02d}"   # nom de fichier conserve ; il designe une repetition
    report = os.path.join(RESULTS_DIR, f"PAIRED_COMPARISON_{task}_{name}.txt")
    if os.path.isfile(report):
        txt = open(report).read()
        a = re.search(r"^A\) irace pur\s*:\s*([-\d.]+)", txt, re.M)
        b = re.search(r"^B\) LGEA LLM\s*:\s*([-\d.]+)", txt, re.M)
        return dict(name=name, etat="terminee",
                    detail=f"A={a.group(1) if a else '?'}  B={b.group(1) if b else '?'}")

    work = os.path.join(RUNS_DIR, task, name)
    if not os.path.isdir(work):
        return dict(name=name, etat="pas commencee", detail="")

    seed_file = os.path.join(work, "execution-folder", "seed_configs.txt")
    parts = []
    parts.append("etape 0 ok" if etape0_done(seed_file) else "etape 0 a refaire")

    score_a, _ = try_extract_irace_best(os.path.join(work, "irace_A.log"))
    parts.append(f"branche A ok ({score_a:.0f})" if score_a is not None
                 else "branche A a refaire")

    state_file = os.path.join(work, "etat_B.json")
    if os.path.isfile(state_file):
        try:
            st = json.load(open(state_file))
            be = st.get("best_ever")
            parts.append(f"branche B au round {st['next_round']}/{N_ROUNDS}"
                         + (f" (meilleur {be[0]:.0f})" if be else ""))
        except Exception:
            parts.append("branche B : etat illisible")
    else:
        n = len([f for f in os.listdir(work) if re.match(r"results_B\d+\.tsv", f)])
        parts.append(f"branche B : {n} round(s) fait(s)" if n else "branche B pas commencee")

    return dict(name=name, etat="interrompue", detail=" | ".join(parts))


def print_status(task: str):
    rows = [mission_status(task, i) for i in range(1, N_REPETITIONS + 1)]
    done = sum(1 for r in rows if r["etat"] == "terminee")
    partial = sum(1 for r in rows if r["etat"] == "interrompue")
    print("=" * 78)
    print(f" AVANCEMENT — {task.upper()} (instance publiee, 10 repetitions)")
    print("=" * 78)
    for r in rows:
        print(f"  repetition-{r['name'].split(chr(45))[-1]}  {r['etat']:14s} {r['detail']}")
    print("-" * 78)
    print(f"  {done}/{N_REPETITIONS} terminees | {partial} interrompue(s) | "
          f"{N_REPETITIONS - done - partial} pas commencee(s)")
    print(f"\n  Relancer reprend ou ca s'est arrete :")
    print(f"    python3 run_paired_comparaison_complet.py {task}")


# ===========================================================================
# UNE COMPARAISON APPARIEE SUR UNE MISSION
# ===========================================================================
def run_one_mission(task, idx, params, irace_bin, meta, irace_params, client):
    mission_name = f"mission-{idx:02d}"   # nom de fichier conserve ; il designe une repetition
    # Instance publiee, identique pour les 10 repetitions.
    instances_dir = os.path.dirname(reference_argos(task))
    argos_file = reference_argos(task)

    work = os.path.join(RUNS_DIR, task, mission_name)
    exec_dir = os.path.join(work, "execution-folder")
    os.makedirs(exec_dir, exist_ok=True)
    api_log = os.path.join(work, "llm_api_errors.log")

    crn_seed = mission_seed(task, idx) if COMMON_RANDOM_NUMBERS else None

    worst = WORST_UNIT_PENALTY.get(task)
    too_bad = (worst * params["robots"] * TICKS_PER_RUN - 1) if worst else None

    print("\n" + "=" * 70)
    print(f" {task.upper()} — {mission_name}")
    print(f" {params['robots']} robots | arene R={params['inner_radius']:.3f} m "
          f"| disque de depart {params['distrib_radius']:.3f} m "
          f"(marge au mur {params['marge_mur']:.3f}) | portee RAB {params['rab']:.2f} m")
    print(f" budget {BUDGET_TOTAL} par branche | firstTest={FIRST_TEST}")
    print("=" * 70)

    # ---------------------------------------------------------------- ETAPE 0
    sc0 = os.path.join(exec_dir, "scenario_0.txt")
    rd0 = os.path.join(exec_dir, "irace_0.Rdata")
    lg0 = os.path.join(work, "irace_0.log")
    tsv0 = os.path.join(work, "results_0.tsv")
    seed_file = os.path.join(exec_dir, "seed_configs.txt")

    if etape0_done(seed_file):
        ranked0 = read_results_tsv(tsv0, meta, irace_params) \
            if os.path.isfile(tsv0) else []
        print(f"\n--- ETAPE 0 : deja faite, on la garde "
              f"({sum(1 for _ in open(seed_file)) - 1} FSMs) ---")
    else:
        print(f"\n--- ETAPE 0 : pool de depart aleatoire (commun aux deux branches) ---")
        for p in (tsv0, lg0):
            if os.path.exists(p):
                os.remove(p)
        write_scenario(sc0, task, exec_dir, instances_dir, budget=BUDGET_PER_ROUND,
                       nb_iterations=1, seed=crn_seed)
        run_irace_round(irace_bin, sc0, rd0, lg0)
        if not extract_round_results(rd0, tsv0):
            print("[ERREUR] irace a echoue a l'etape 0. Mission ignoree.")
            return None
        ranked0 = read_results_tsv(tsv0, meta, irace_params)
        if not ranked0:
            print("[ERREUR] aucune configuration aleatoire exploitable. Mission ignoree.")
            return None
        write_configurations_file(seed_file, irace_params, [d for _, _, d, _ in ranked0])
        print(f"[OK] {len(ranked0)} FSMs aleatoires generees.")

    # -------------------------------------------------------------- BRANCHE A
    print(f"\n--- BRANCHE A : irace continu ({N_ROUNDS} iterations, budget {BUDGET_TOTAL}) ---")
    sca = os.path.join(exec_dir, "scenario_A.txt")
    rda = os.path.join(exec_dir, "irace_A.Rdata")
    lga = os.path.join(work, "irace_A.log")
    if branche_a_terminee(lga):
        score_a, cli_a = try_extract_irace_best(lga)
        print(f"[REPRISE] branche A deja terminee, on la garde "
              f"(score {score_a:.2f}) — {BUDGET_TOTAL} evaluations economisees.")
    else:
        if os.path.isfile(lga):
            done = parse_irace_iterations(lga)
            print(f"[REPRISE] branche A incomplete "
                  f"({len(done)}/{N_ROUNDS} iterations) — on la relance en entier.")
        write_scenario(sca, task, exec_dir, instances_dir, budget=BUDGET_TOTAL,
                       nb_iterations=N_ROUNDS, config_file=seed_file, seed=crn_seed)
        run_irace_round(irace_bin, sca, rda, lga)
        score_a, cli_a = try_extract_irace_best(lga)
        print(f"[OK] Branche A terminee — score d'entrainement "
              f"{'%.2f' % score_a if score_a is not None else 'N/A'}")

    # -------------------------------------------------------------- BRANCHE B
    print(f"\n--- BRANCHE B : LGEA conversationnel ({N_ROUNDS} rounds, budget {BUDGET_TOTAL}) ---")
    system_prompt = build_prompt_context(task, meta, params)
    with open(os.path.join(work, "system_prompt.txt"), "w") as f:
        f.write(system_prompt)

    state_file = os.path.join(work, "etat_B.json")
    messages = []
    pool_file = seed_file
    remaining_budget = BUDGET_TOTAL
    best_ever = None
    llm_pool_keys = set()
    history = []
    start_round = 1

    if os.path.isfile(state_file):
        try:
            st = json.load(open(state_file))
            start_round = st["next_round"]
            messages = st["messages"]
            pool_file = st["pool_file"]
            remaining_budget = st["remaining_budget"]
            best_ever = tuple(st["best_ever"]) if st["best_ever"] else None
            llm_pool_keys = {tuple(k) for k in st["llm_pool_keys"]}
            history = st["history"]
            print(f"[REPRISE] branche B reprise au round {start_round}/{N_ROUNDS} "
                  f"(budget restant {remaining_budget}, meilleur "
                  f"{best_ever[0]:.2f})" if best_ever else
                  f"[REPRISE] branche B reprise au round {start_round}/{N_ROUNDS}")
        except Exception as e:
            print(f"[DIAG] etat B illisible ({e}) — la branche B repart du round 1.")
            start_round, messages = 1, []

    for r in range(start_round, N_ROUNDS + 1):
        budget_r = max(1, remaining_budget // (N_ROUNDS - r + 1))
        remaining_budget -= budget_r
        print(f"\n  --- Round {r}/{N_ROUNDS} — budget {budget_r} ---")

        scb = os.path.join(exec_dir, f"scenario_B{r}.txt")
        rdb = os.path.join(exec_dir, f"irace_B{r}.Rdata")
        lgb = os.path.join(work, f"irace_B{r}.log")
        tsvb = os.path.join(work, f"results_B{r}.tsv")
        if os.path.exists(tsvb):
            os.remove(tsvb)

        write_scenario(scb, task, exec_dir, instances_dir, budget=budget_r,
                       nb_iterations=1, config_file=pool_file,
                       seed=(None if crn_seed is None else crn_seed + (r - 1)))
        run_irace_round(irace_bin, scb, rdb, lgb)
        if not extract_round_results(rdb, tsvb):
            print(f"[ERREUR] round {r} en echec. Mission ignoree.")
            return None

        ranked = read_results_tsv(tsvb, meta, irace_params)
        nb_conf = parse_nb_configurations(lgb)
        top = ranked[0]

        origin = ("LLM" if config_key(top[2], irace_params) in llm_pool_keys
                  else ("irace" if r > 1 else "pool-initial"))
        if best_ever is None or top[0] < best_ever[0]:
            best_ever = (top[0], top[1], origin, r)

        print(f"   [SCORE] round {top[0]:.2f} ({origin}, {top[3]} evaluations) "
              f"| meilleur global {best_ever[0]:.2f} ({best_ever[2]}, round {best_ever[3]})")
        history.append(f"round {r:2d} | best={top[0]:9.2f} | origine={origin:12s} "
                       f"| nbConfigurations={nb_conf:3d} | pool LLM={len(llm_pool_keys):3d} "
                       f"| budget={budget_r}")

        if r == N_ROUNDS:
            break

        target_next = max(MIN_LLM_BATCH_PER_ROUND, min(MAX_LLM_BATCH_PER_ROUND, nb_conf))
        accepted, retry = [], 0

        while len(accepted) < MIN_LLM_BATCH_PER_ROUND and retry < MAX_RETRIES:
            if retry == 0:
                messages.append({"role": "user", "content": build_round_message(
                    task, r, budget_r, [(s, c) for s, c, _, _ in ranked],
                    best_ever[:2] if best_ever else None, target_next, work)})
                print(f"   [LLM] generation de la population du round {r+1}...", flush=True)
            else:
                messages.append({"role": "user", "content":
                    f"Your last answer yielded only {len(accepted)} usable machines. "
                    f"Give exactly {target_next} lines starting with 'FSM: ', one per line, "
                    f"with no numbering and no bold markup around the prefix. "
                    f"Keep the reasoning to two sentences per machine so you have room "
                    f"to finish every line."})
                print(f"   [RETRY] relance du LLM ({retry+1}/{MAX_RETRIES})...", flush=True)

            text, _ = call_llm(client, system_prompt, messages, target_next, api_log)
            messages.append({"role": "assistant", "content": text})

            cands = extract_all_fsm_lines(text)
            print(f"   [DIAG] {len(cands)} ligne(s) 'FSM: ' extraite(s).", flush=True)
            if not cands and text:
                dbg = os.path.join(work, f"last_failed_response_B{r}.txt")
                with open(dbg, "w") as f:
                    f.write(text)
                print(f"   [DIAG] aucune ligne 'FSM: ' -> reponse brute dans {dbg}", flush=True)

            errs_round, n_deg, n_dup, n_crash, n_bad = [], 0, 0, 0, 0
            for cli in cands:
                d, errs = cli_to_irace_dict(cli, task, meta, irace_params)
                errs_round.extend(errs)

                ok, why = validate_topology(d)
                if not ok:
                    n_deg += 1
                    errs_round.append(f"A machine was rejected before simulation: {why}.")
                    continue
                if any(d == a[1] for a in accepted):
                    n_dup += 1
                    continue

                if PRE_SCREEN:
                    score = evaluate_fsm(cli, argos_file, PRE_SCREEN_SEEDS)
                    remaining_budget -= len(PRE_SCREEN_SEEDS)   # budget decompte
                    if score is None:
                        n_crash += 1
                        continue
                    if too_bad is not None and score >= too_bad:
                        n_bad += 1
                        continue
                accepted.append((cli, d))

            if errs_round:
                save_error_memory(task, load_error_memory(task, work) + errs_round, work)
            print(f"   [DIAG] {len(accepted)} acceptee(s), {n_deg} degeneree(s), "
                  f"{n_dup} doublon(s), {n_crash} crash/timeout, {n_bad} hors seuil.",
                  flush=True)
            retry += 1

        if not accepted:
            print("   [FATAL] aucune FSM exploitable apres "
                  f"{MAX_RETRIES} essais. Mission ignoree.")
            return None

        accepted = accepted[:target_next]
        pool_file = os.path.join(exec_dir, f"llm_pool_B{r+1}.txt")
        write_configurations_file(pool_file, irace_params, [d for _, d in accepted])
        llm_pool_keys = {config_key(d, irace_params) for _, d in accepted}
        save_state_b(state_file, r + 1, messages, pool_file, remaining_budget,
                     best_ever, llm_pool_keys, history)
        print(f"   [OK] {len(accepted)} FSMs LLM retenues pour le round {r+1}.")
        if len(accepted) < nb_conf:
            print(f"   [ATTENTION] irace completera {nb_conf - len(accepted)} "
                  f"configuration(s) au hasard au round {r+1}.")

    # ---------------------------------------------------------------- RAPPORT
    score_a_final = score_a if score_a is not None else float("inf")
    score_b_final = best_ever[0] if best_ever else float("inf")
    cli_b = best_ever[1] if best_ever else "N/A"

    print("\n" + "=" * 70)
    print(f"  {task.upper()} / {mission_name}")
    print(f"  A) irace pur (score entrainement) : {score_a_final:.2f}")
    print(f"  B) LGEA LLM  (score entrainement) : {score_b_final:.2f}   "
          f"(origine : {best_ever[2] if best_ever else 'N/A'})")
    print("=" * 70)

    os.makedirs(RESULTS_DIR, exist_ok=True)
    report = os.path.join(RESULTS_DIR, f"PAIRED_COMPARISON_{task}_{mission_name}.txt")
    with open(report, "w") as f:
        f.write(f"COMPARAISON APPARIEE — tache {task.upper()} / {mission_name}\n")
        f.write("=" * 80 + "\n\nPARAMETRES DE LA MISSION\n" + "-" * 80 + "\n")
        for k in ("edges", "boxes", "length", "inner_radius", "robots", "density",
                  "radius_published", "distrib_ratio", "distrib_radius",
                  "marge_mur", "rab"):
            f.write(f"  {k:15s} : {params[k]}\n")
        f.write(f"\nBudget par branche : {BUDGET_TOTAL} | firstTest : {FIRST_TEST} "
                f"| rounds : {N_ROUNDS}\n")
        f.write(f"Pool de depart     : {len(ranked0)} FSMs aleatoires\n")
        f.write(f"Pre-filtrage simule: {PRE_SCREEN} "
                f"({len(PRE_SCREEN_SEEDS)} graines, decomptees du budget B)\n")
        f.write("\n" + "=" * 80 + "\nSCORES D'ENTRAINEMENT FINAUX\n" + "-" * 80 + "\n")
        f.write(f"A) irace pur : {score_a_final:.2f}\n")
        f.write(f"B) LGEA LLM  : {score_b_final:.2f}   (origine : "
                f"{best_ever[2] if best_ever else 'N/A'}, round {best_ever[3] if best_ever else '-'})\n\n")
        f.write(f"Meilleure FSM (branche A) :\n{cli_a}\n\n")
        f.write(f"Meilleure FSM (branche B) :\n{cli_b}\n\n")
        f.write("=" * 80 + "\n")
        f.write("HISTORIQUE — BRANCHE A (irace pur, une ligne par iteration)\n")
        f.write("-" * 80 + "\n")
        f.write(format_history_a(parse_irace_iterations(lga)))
        f.write("\n" + "=" * 80 + "\n")
        f.write("HISTORIQUE — BRANCHE B (LGEA LLM, une ligne par round)\n")
        f.write("-" * 80 + "\n")
        f.write("\n".join(history) + "\n")
    print(f"[SAVE] {report}")
    for f_ in export_mission_logs(task, idx, params):
        print(f"[SAVE] {f_}")

    return dict(mission=mission_name, score_a=score_a_final, score_b=score_b_final,
                origin_b=best_ever[2] if best_ever else "N/A",
                robots=params["robots"], rab=params["rab"],
                radius=params["inner_radius"],
                distrib_radius=params["distrib_radius"])


# ===========================================================================
# RECAPITULATIF
# ===========================================================================
def wilcoxon(diffs):
    nz = [d for d in diffs if d != 0]
    if not nz:
        return None, 0
    order = sorted(range(len(nz)), key=lambda i: abs(nz[i]))
    ranks = [0.0] * len(nz)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and abs(nz[order[j + 1]]) == abs(nz[order[i]]):
            j += 1
        avg = (i + j) / 2 + 1
        for k in range(i, j + 1):
            ranks[order[k]] = avg
        i = j + 1
    wp = sum(r for r, d in zip(ranks, nz) if d > 0)
    wn = sum(r for r, d in zip(ranks, nz) if d < 0)
    return min(wp, wn), len(nz)


def write_recap(task, rows):
    if not rows:
        return
    os.makedirs(RESULTS_DIR, exist_ok=True)
    path = os.path.join(RESULTS_DIR, f"RECAP_{task}.tsv")
    with open(path, "w") as f:
        f.write("mission\trobots\trab\tradius\tdistrib_radius\tscore_A\tscore_B\t"
                "diff_B_moins_A\torigine_B\n")
        for r in rows:
            f.write(f"{r['mission']}\t{r['robots']}\t{r['rab']}\t{r['radius']}\t"
                    f"{r['distrib_radius']}\t{r['score_a']:.4f}\t{r['score_b']:.4f}\t"
                    f"{r['score_b'] - r['score_a']:.4f}\t{r['origin_b']}\n")

    diffs = [r["score_b"] - r["score_a"] for r in rows]
    wins_b = sum(1 for d in diffs if d < 0)
    from_llm = sum(1 for r in rows if r["origin_b"] == "LLM")
    w, n = wilcoxon(diffs)
    mean = lambda xs: sum(xs) / len(xs)
    med = lambda xs: sorted(xs)[len(xs) // 2] if len(xs) % 2 else \
        (sorted(xs)[len(xs) // 2 - 1] + sorted(xs)[len(xs) // 2]) / 2

    print("\n" + "=" * 70)
    print(f" RECAPITULATIF — {task.upper()}  ({len(rows)} missions)")
    print("=" * 70)
    print(f"  A (irace) moyenne      : {mean([r['score_a'] for r in rows]):.2f}")
    print(f"  B (LLM)   moyenne      : {mean([r['score_b'] for r in rows]):.2f}")
    print(f"  victoires de B         : {wins_b}/{len(rows)}")
    print(f"  mediane(B - A)         : {med(diffs):+.2f}")
    print(f"  champions B issus du LLM : {from_llm}/{len(rows)} "
          f"(le reste vient de tirages d'irace)")
    print(f"  Wilcoxon apparie       : W={w} sur n={n} paires non nulles")
    if n < 6:
        print("  (n < 6 : aucune conclusion statistique possible)")
    print(f"\n  detail : {path}")
    print("\n  Rappel : ce sont des scores d'ENTRAINEMENT. L'elite d'irace est moyennee")
    print("  sur beaucoup plus d'evaluations que le meilleur d'un round LLM, donc la")
    print("  comparaison penche en faveur de B. Pour un verdict publiable, rejoue les")
    print("  deux champions sur des graines de test disjointes.")


# ===========================================================================
def parse_repetitions_arg(spec: str) -> list:
    out = set()
    for part in spec.split(","):
        part = part.strip()
        if "-" in part:
            a, b = part.split("-")
            out.update(range(int(a), int(b) + 1))
        elif part:
            out.add(int(part))
    return sorted(i for i in out if 1 <= i <= N_REPETITIONS)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("task", choices=TASKS)
    ap.add_argument("--repetitions", default=f"1-{N_REPETITIONS}",
                    help="sous-ensemble, ex. '1-5' ou '3,7' (defaut : toutes)")
    ap.add_argument("--redo", action="store_true",
                    help="refaire les missions dont le rapport existe deja")
    ap.add_argument("--status", action="store_true",
                    help="afficher l'avancement et sortir, sans rien lancer")
    args = ap.parse_args()

    task = args.task

    if args.status:
        print_status(task)
        return

    check_base()
    check_api_key()

    wanted = parse_repetitions_arg(args.repetitions)

    print("=" * 70)
    print(f" COMPARAISON APPARIEE — {task.upper()}")
    print(f" {len(wanted)} repetition(s) | budget {BUDGET_TOTAL} par branche "
          f"| firstTest={FIRST_TEST}")
    print(f" instance : {reference_argos(task)} (publiee, sans variation)")
    print("=" * 70)
    print(f"\n[INFO] {len(wanted)} repetitions x 2 branches x {BUDGET_TOTAL} evaluations")
    print(f"       = {len(wanted) * 2 * BUDGET_TOTAL} simulations ARGoS pour cette tache.")
    print("       Compte plusieurs jours en sequentiel. Tu peux decouper avec")
    print("       --repetitions 1-5, --repetitions 6-10, etc. et lancer en parallele.\n")

    irace_bin = get_irace_bin()
    _, irace_params, meta = load_grammar(task)
    client = anthropic.Anthropic(api_key=ANTHROPIC_API_KEY)

    rows = []
    for idx in wanted:
        report = os.path.join(RESULTS_DIR,
                              f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
        if os.path.isfile(report) and not args.redo:
            print(f"\n[SKIP] mission-{idx:02d} : rapport deja present "
                  f"(--redo pour refaire).")
            continue
        res = run_one_mission(task, idx, published_params(task), irace_bin,
                              meta, irace_params, client)
        if res:
            rows.append(res)

    # le recapitulatif reprend aussi les missions faites lors d'un lancement precedent
    for idx in range(1, N_REPETITIONS + 1):
        if any(r["mission"] == f"mission-{idx:02d}" for r in rows):
            continue
        report = os.path.join(RESULTS_DIR,
                              f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
        if not os.path.isfile(report):
            continue
        txt = open(report).read()
        a = re.search(r"^A\) irace pur\s*:\s*([-\d.]+)", txt, re.M)
        b = re.search(r"^B\) LGEA LLM\s*:\s*([-\d.]+)\s*\(origine : (\w+)", txt, re.M)
        if a and b:
            p = published_params(task)
            rows.append(dict(mission=f"mission-{idx:02d}", score_a=float(a.group(1)),
                             score_b=float(b.group(1)), origin_b=b.group(2),
                             robots=p["robots"], rab=p["rab"],
                             radius=p["inner_radius"],
                             distrib_radius=p["distrib_radius"]))

    rows.sort(key=lambda r: r["mission"])
    write_recap(task, rows)


if __name__ == "__main__":
    main()

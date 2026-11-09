#!/usr/bin/env python3
"""
run_paired_comparison_d_origin.py — BRAS TEMOIN

Reprise a l'identique du protocole d'origine (etape 0 -> branche A -> branche B
en 10 rounds), adapte au banc de missions de LLM_Mission. Il tourne sur LES
MEMES instances que le bras courant, mais avec l'ancien module : ancien prompt,
ancien catalogue de conditions, filtre de graphe fortement connexe, et
pre-filtrage a 3 graines NON decompte du budget.

Sorties separees : resultats_origin/ et runs_origin/.

Usage :
    python3 run_paired_comparison_d_origin.py aggregation
    python3 run_paired_comparison_d_origin.py stop --missions 1-5
    python3 run_paired_comparison_d_origin.py aggregation --status

ATTENTION : la memoire d'erreurs du LLM est PARTAGEE par tache, comme dans la
version d'origine. Ne lance pas deux processus sur la meme tache en parallele :
ils s'ecraseraient. Des taches differentes en parallele, aucun probleme.
"""

_ANCIEN_DOCSTRING = """
run_paired_comparison_d.py

Comparaison APPARIEE ("Test A/B Strict") sans evaluation hors-ligne finale :
1. ETAPE 0 : "Mini-irace" pour generer ~15 FSMs totalement ALEATOIRES.
2. BRANCHE A : irace continu (6000 evals). Demarre avec les FSMs de l'Etape 0.
3. BRANCHE B : LGEA Conversationnel. (Rounds 2-10 guides 100% LLM avec prompts NLP)
4. VERDICT   : Comparaison s'appuyant uniquement sur les scores d'entrainement finaux.
"""

import sys
import argparse
import os
import re
import time
import subprocess
import anthropic

import generate_missions as gm
# le parseur d'iterations vit dans le bras courant : on le reutilise
# plutot que d'en ecrire un second
from run_paired_comparison_d_missions import (parse_irace_iterations,
                                             format_history_a)

from automode_llm_common_origin import (
    check_api_key, load_grammar, build_prompt_context, ANTHROPIC_API_KEY,
    cli_to_irace_dict, evaluate_fsm, EXE, ARGOS,
    write_configurations_file, BASE, GRAMMAR_FILES,
    load_error_memory, save_error_memory, fix_transition_destinations,
    validate_scg_topology,
    # ajouts propres au dossier mission
    TASKS, MISSIONS_DIR, RESULTS_DIR, RUNS_DIR, LLM_DIR,
    mission_dir, mission_argos, check_base,
)

N_ROUNDS = 10           
BUDGET_TOTAL = 6000     
BUDGET_PER_ROUND = BUDGET_TOTAL // N_ROUNDS
FIRST_TEST = 40

MAX_LLM_BATCH_PER_ROUND = 20
MIN_LLM_BATCH_PER_ROUND = 3

LLM_DIR = os.path.dirname(os.path.abspath(__file__))
# RESULTS_DIR et RUNS_DIR viennent du module origin (resultats_origin/, runs_origin/)

MULTI_FSM_ADDENDUM = """
# 5. CONVERSATION MODE — YOU ARE THE EVOLUTIONARY ALGORITHM
This is an ongoing conversation across several rounds of real-world ARGoS3 optimization.
Because the mathematical optimizer's internal memory is reset at the start of every round, YOU are solely responsible for driving the evolution. You must manually perform both EXPLORATION (finding new ideas) and EXPLOITATION (refining good ideas).

For every round, you MUST follow these four evolutionary rules:
1. **SURVIVAL (Elitism):** You must explicitly copy the exact 'Best-ever' FSM in your response so it survives to the next round. If you forget it, the lineage is lost!
2. **EXPLOITATION (Fine-tuning):** Dedicate about half of your newly generated FSMs to slightly mutate the 'Best-ever' and the 'Best performers of this round'. Tweak transition probabilities (--p), thresholds (--b/--w), or speeds slightly (e.g., +/- 10%). Do NOT drastically change their structural topology.
3. **EXPLORATION:** Dedicate the remaining FSMs to testing completely new topological structures or novel state combinations to escape local optima.
4. **BRANCHING DIVERSITY:** Do not converge on FSMs where every state has only one simple transition on the same condition type. Include some candidates with 2-4 transitions per state mixing different condition types (e.g. a floor-color condition AND a neighbor-count condition AND a time-in-state condition out of the same state). irace's random sampling explores this kind of richer branching by default; matching that richness in at least some of your candidates is necessary for a fair comparison.
"""

def get_irace_bin() -> str:
    r = subprocess.run(["Rscript", "-e", 'cat(system.file("bin", "irace", package="irace"))'], capture_output=True, text=True)
    path = r.stdout.strip()
    if not path or not os.path.isfile(path): 
        print("[ERREUR] irace introuvable.")
        sys.exit(1)
    return path

def irace_paths(task: str, idx: int) -> dict:
    """Memes champs qu'avant, mais pointes sur UNE mission du banc.

    Seule adaptation au dossier LLM_Mission : les instances ne sont plus
    l'unique experiments-folder du depot, et le dossier d'execution est propre
    a la mission pour que plusieurs missions puissent tourner en parallele.
    """
    irace_task_dir = f"{BASE}/irace-files/irace-{task}"
    work = os.path.join(RUNS_DIR, task, f"mission-{idx:02d}")
    return {
        "exec_dir": os.path.join(work, "execution-folder"),
        "work": work,
        "grammar": GRAMMAR_FILES[task],
        "instances_dir": mission_dir(task, idx),
        "target_runner": f"{irace_task_dir}/target-runner",
    }

def write_scenario(path, grammar, exec_dir, instances_dir, target_runner, budget, nb_iterations=None, config_file=None):
    os.makedirs(exec_dir, exist_ok=True)
    
    lines = [
        f'parameterFile     = "{grammar}"',
        f'execDir           = "{exec_dir}"',
        f'trainInstancesDir = "{instances_dir}"',
        f'targetRunner      = "{target_runner}"',
        f'maxExperiments    = {budget}',
        f'firstTest         = {FIRST_TEST}',
        f'mu                = 5',
    ]
    if nb_iterations is not None: lines.append(f'nbIterations      = {nb_iterations}')
    if config_file: lines.append(f'configurationsFile = "{config_file}"')
    
    with open(path, "w") as f: f.write("\n".join(lines) + "\n")

def run_irace_round(irace_bin, scenario_path, rdata_path, log_path) -> bool:
    os.makedirs(os.path.dirname(log_path), exist_ok=True)
    with open(log_path, "w") as logf:
        proc = subprocess.Popen([irace_bin, "--scenario", scenario_path, "--log-file", rdata_path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
        for line in proc.stdout:
            print(line, end="", flush=True)
            logf.write(line)
        proc.wait()
    return proc.returncode == 0 and os.path.isfile(rdata_path)

def parse_nb_configurations(log_path: str):
    if not os.path.exists(log_path): return MAX_LLM_BATCH_PER_ROUND
    try:
        with open(log_path, "r") as f: text = f.read()
        matches = re.findall(r"#\s*nbConfigurations:\s*(\d+)", text)
        return int(matches[0]) if matches else MAX_LLM_BATCH_PER_ROUND
    except Exception: return MAX_LLM_BATCH_PER_ROUND

def try_extract_irace_best(log_path: str):
    if not os.path.isfile(log_path): return None, None
    with open(log_path) as f: text = f.read()
    
    matches_score = re.findall(r"Best-so-far configuration:\s*\d+\s*mean value:\s*([\d.-]+)", text)
    score = float(matches_score[-1]) if matches_score else None
    
    cli = None
    lines = text.split("\n")
    capture = False
    
    for line in lines:
        if "# Best configurations as commandlines" in line:
            capture = True
            continue
        if capture and "--nstates" in line and not line.startswith("#"):
            cli = " ".join(line.split()[1:])
            break
            
    return score, cli

def extract_round_results(r_script: str, rdata_path: str, tsv_path: str) -> bool:
    proc = subprocess.run(["Rscript", r_script, rdata_path, tsv_path], capture_output=True, text=True)
    return proc.returncode == 0 and os.path.isfile(tsv_path)

def rebuild_cli_from_dict(row_dict, param_meta, irace_params):
    tokens = [f"--nstates {row_dict.get('NumStates', '1')}"]
    for p in irace_params:
        val = row_dict.get(p, "NA")
        if p != "NumStates" and val != "NA":
            if p in param_meta: tokens.append(f"{param_meta[p]['switch']} {val}")
    return " ".join(tokens)

def read_results_tsv(tsv_path, param_meta, irace_params) -> list:
    results = []
    with open(tsv_path) as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            if not line.strip(): continue
            fields = line.rstrip("\n").split("\t")
            row = dict(zip(header, fields))
            try: score = float(row.get("MeanCost", "NA"))
            except ValueError: continue
            d = {p: row.get(p, "NA") for p in irace_params}
            cli = rebuild_cli_from_dict(row, param_meta, irace_params)
            results.append((score, cli, d))
    results.sort(key=lambda x: x[0])
    return results

def extract_all_fsm_lines(text: str) -> list:
    out = []
    for raw_line in text.split("\n"):
        line = raw_line.strip()
        line = re.sub(r'^[\*\-\u2022]\s*', '', line)
        line = re.sub(r'^\d+[\.\)]\s*', '', line)
        line = line.strip('*_ ')
        if line.startswith("FSM:"):
            out.append(line.replace("FSM:", "", 1).strip())
    return out

def fmt_entry(e) -> str:
    return f"Score: {e[0]:.2f} -> FSM: {e[1]}"

def build_round_message(task, round_no, n_rounds, budget_r, ranked_round, best_ever, target_next):
    best = ranked_round[:5]
    worst = ranked_round[-3:] if len(ranked_round) > 5 else []
    best_block = "\n".join(f"  {i + 1}. {fmt_entry(e)}" for i, e in enumerate(best))
    worst_block = "\n".join(f"  {i + 1}. {fmt_entry(e)}" for i, e in enumerate(worst)) if worst else "(not enough data)"
    best_ever_line = f"Score: {best_ever[0]:.2f} -> FSM: {best_ever[1]}" if best_ever else "(none yet)"

    msg = (
        f"Round {round_no} out of {n_rounds} has just completed on the {task.upper()} task.\n"
        f"As a reminder, irace MINIMIZES the fitness score, meaning a LOWER score is always better.\n\n"
        f"=== CURRENT BEST EVER (THE CHAMPION) ===\n"
        f"You MUST include an exact copy of this FSM in your next response so it survives:\n"
        f"  {best_ever_line}\n\n"
        f"=== BEST PERFORMERS THIS ROUND ===\n"
        f"These FSMs did well. Use them as parents for your 'Exploitation' micro-mutations:\n"
        f"{best_block}\n\n"
        f"=== WORST PERFORMERS THIS ROUND ===\n"
        f"These FSMs failed. Avoid these structures and parameter combinations:\n"
        f"{worst_block}\n\n"
        f"=== YOUR TASK FOR ROUND {round_no + 1} ===\n"
        f"You must supply exactly {target_next} FSMs to form the complete population for the next generation.\n"
        f"Remember: Output your reasoning first, and then list each FSM on its own line strictly starting with 'FSM: '.\n"
    )
    persistent_errors = load_error_memory(task)
    if persistent_errors:
        msg += "\n[WARNING] CRITICAL PREVIOUS ERRORS TO AVOID THIS TIME:\n"
        for err in persistent_errors[-10:]: msg += f"- {err}\n"
    return msg



# ===========================================================================
# UNE COMPARAISON APPARIEE SUR UNE MISSION
# Meme enchainement que le main() d'origine : etape 0, branche A, branche B.
# ===========================================================================
def run_one_mission(task, idx, irace_bin, irace_params, param_meta,
                    system_prompt, client, r_script):
    paths = irace_paths(task, idx)
    argos_file = mission_argos(task, idx)
    work = paths["work"]
    os.makedirs(work, exist_ok=True)

    print("=" * 64)
    print(f" COMPARAISON APPARIEE STRICTE (LGEA vs Baseline irace) — BRAS TEMOIN")
    print(f" Tache : {task:<12} |  mission-{idx:02d}  |  Budget total : {BUDGET_TOTAL}")
    print("=" * 64)

    # =========================================================================
    # ETAPE 0 : GENERER LA LIGNE DE DEPART ALEATOIRE
    # =========================================================================
    print(f"\n{'=' * 65}\nETAPE 0 — Creation du Pool de Depart Aleatoire (irace)\n{'=' * 65}")
    scenario_0 = f"{paths['exec_dir']}/scenario_paired_0_{task}.txt"
    rdata_0 = f"{paths['exec_dir']}/irace_paired_0_{task}.Rdata"
    log_0 = f"{work}/irace_paired_0_{task}.log"
    tsv_0 = f"{work}/paired_0_results_{task}.tsv"
    for p in (tsv_0, log_0):
        if os.path.exists(p):
            os.remove(p)

    write_scenario(scenario_0, paths["grammar"], paths["exec_dir"],
                   paths["instances_dir"], paths["target_runner"],
                   budget=BUDGET_PER_ROUND, nb_iterations=1)
    run_irace_round(irace_bin, scenario_0, rdata_0, log_0)
    if not extract_round_results(r_script, rdata_0, tsv_0):
        print("[ERREUR] irace a echoue a l'etape 0. Mission ignoree.")
        return None

    ranked_0 = read_results_tsv(tsv_0, param_meta, irace_params)
    if not ranked_0:
        print("[ERREUR] aucune configuration aleatoire exploitable. Mission ignoree.")
        return None
    n_seeds_generated = len(ranked_0)

    seed_file = f"{paths['exec_dir']}/seed_configs_{task}.txt"
    write_configurations_file(seed_file, irace_params, [d for _, _, d in ranked_0])
    print(f"[OK] {n_seeds_generated} FSMs aleatoires generees.")

    # =========================================================================
    # BRANCHE A : BASELINE IRACE CONTINU
    # =========================================================================
    print(f"\n{'=' * 65}\nBRANCHE A — irace continu ({N_ROUNDS} iterations, budget {BUDGET_TOTAL})\n{'=' * 65}")
    scenario_a = f"{paths['exec_dir']}/scenario_paired_A_{task}.txt"
    rdata_a = f"{paths['exec_dir']}/irace_paired_A_{task}.Rdata"
    log_a = f"{work}/irace_paired_A_{task}.log"

    write_scenario(scenario_a, paths["grammar"], paths["exec_dir"],
                   paths["instances_dir"], paths["target_runner"],
                   budget=BUDGET_TOTAL, nb_iterations=N_ROUNDS,
                   config_file=seed_file)
    run_irace_round(irace_bin, scenario_a, rdata_a, log_a)
    score_a_train, cli_a = try_extract_irace_best(log_a)
    print(f"[OK] Branche A terminee — score d'entrainement "
          f"{'%.2f' % score_a_train if score_a_train is not None else 'N/A'}")

    # =========================================================================
    # BRANCHE B : LGEA (Boucle LLM - PURE)
    # =========================================================================
    print(f"\n{'=' * 65}\nBRANCHE B — LGEA Conversationnel ({N_ROUNDS} rounds, budget {BUDGET_TOTAL})\n{'=' * 65}")
    messages = []
    pool_file = seed_file
    remaining_budget = BUDGET_TOTAL
    best_ever = None
    round_history_log = []

    for r in range(1, N_ROUNDS + 1):
        budget_r = max(1, remaining_budget // (N_ROUNDS - r + 1))
        remaining_budget -= budget_r
        print(f"\n  --- Round {r}/{N_ROUNDS} — budget {budget_r} ---")

        scenario_b = f"{paths['exec_dir']}/scenario_paired_B{r}_{task}.txt"
        rdata_b = f"{paths['exec_dir']}/irace_paired_B{r}_{task}.Rdata"
        log_b = f"{work}/irace_paired_B{r}_{task}.log"
        tsv_b = f"{work}/paired_B{r}_results_{task}.tsv"
        if os.path.exists(tsv_b):
            os.remove(tsv_b)

        write_scenario(scenario_b, paths["grammar"], paths["exec_dir"],
                       paths["instances_dir"], paths["target_runner"],
                       budget=budget_r, nb_iterations=1, config_file=pool_file)
        run_irace_round(irace_bin, scenario_b, rdata_b, log_b)
        if not extract_round_results(r_script, rdata_b, tsv_b):
            print(f"[ERREUR] round {r} en echec. Mission ignoree.")
            return None

        ranked_round = read_results_tsv(tsv_b, param_meta, irace_params)
        nb_conf = parse_nb_configurations(log_b)
        top = ranked_round[0]
        if best_ever is None or top[0] < best_ever[0]:
            best_ever = (top[0], top[1], top[2])

        print(f"   [SCORE] round {top[0]:.2f} | meilleur global {best_ever[0]:.2f}")
        round_history_log.append(
            f"round {r:2d} | best={top[0]:9.2f} | origine=non-suivi   "
            f"| nbConfigurations={nb_conf:3d} | budget={budget_r}")

        if r == N_ROUNDS:
            break

        target_next = max(MIN_LLM_BATCH_PER_ROUND,
                          min(MAX_LLM_BATCH_PER_ROUND, nb_conf))
        accepted = []
        retry = 0
        while len(accepted) < MIN_LLM_BATCH_PER_ROUND and retry < 3:
            if retry == 0:
                messages.append({"role": "user", "content": build_round_message(
                    task, r, N_ROUNDS, budget_r,
                    [(s, c) for s, c, _ in ranked_round],
                    (best_ever[0], best_ever[1]) if best_ever else None,
                    target_next)})
                print(f"   [LLM] generation de la population du round {r+1}...", flush=True)
            else:
                messages.append({"role": "user", "content":
                    f"Your last answer yielded only {len(accepted)} usable machines. "
                    f"Give exactly {target_next} lines starting with 'FSM: ', one per "
                    f"line, with no numbering and no bold markup."})
                print(f"   [RETRY] relance du LLM ({retry+1}/3)...", flush=True)

            try:
                resp = client.messages.create(
                    model="claude-sonnet-4-6",
                    max_tokens=450 * target_next + 1000,
                    temperature=0.9, system=system_prompt, messages=messages)
                text = resp.content[0].text
            except Exception as e:
                print(f"   [DIAG] appel API en echec : {type(e).__name__}: {e}", flush=True)
                text = ""
            messages.append({"role": "assistant", "content": text})

            cands = extract_all_fsm_lines(text)
            print(f"   [DIAG] {len(cands)} ligne(s) 'FSM: ' extraite(s).", flush=True)

            errs_round, n_deg, n_dup, n_bad = [], 0, 0, 0
            for cli in cands:
                d, errs = cli_to_irace_dict(cli, task, param_meta, irace_params)
                errs_round.extend(errs)

                ok, why = validate_scg_topology(d)
                if not ok:
                    n_deg += 1
                    errs_round.append(f"A machine was rejected before simulation: {why}.")
                    continue
                if any(d == a[1] for a in accepted):
                    n_dup += 1
                    continue

                # Pre-filtrage a 3 graines, NON decompte du budget : c'est le
                # comportement d'origine, conserve volontairement.
                score = evaluate_fsm(cli, task, argos_file)
                if score is None or score >= 23999:
                    n_bad += 1
                    continue
                accepted.append((cli, d, score))

            if errs_round:
                save_error_memory(task, load_error_memory(task) + errs_round)
            print(f"   [DIAG] {len(accepted)} acceptee(s), {n_deg} degeneree(s), "
                  f"{n_dup} doublon(s), {n_bad} ecartee(s) par la simulation.", flush=True)
            retry += 1

        if not accepted:
            print("   [FATAL] aucune FSM exploitable apres 3 essais. Mission ignoree.")
            return None

        accepted.sort(key=lambda a: a[2])
        accepted = accepted[:target_next]
        pool_file = f"{paths['exec_dir']}/llm_pool_paired_B{r+1}_{task}.txt"
        write_configurations_file(pool_file, irace_params, [d for _, d, _ in accepted])
        print(f"   [OK] {len(accepted)} FSMs LLM retenues pour le round {r+1}.")

    # =========================================================================
    # VERDICT ET RAPPORT
    # =========================================================================
    score_a_final = score_a_train if score_a_train is not None else float("inf")
    score_b_final = best_ever[0] if best_ever else float("inf")
    cli_b = best_ever[1] if best_ever else "N/A"

    print("\n" + "=" * 64)
    print(f"       VERDICT — {task.upper()} / mission-{idx:02d}  (bras temoin)")
    print("=" * 64)
    print(f"  A) irace pur (Meilleur Score Entrainement)   : {score_a_final:.2f}")
    print(f"  B) LGEA LLM  (Meilleur Score Entrainement)   : {score_b_final:.2f}")
    print("=" * 64)

    os.makedirs(RESULTS_DIR, exist_ok=True)
    report_path = os.path.join(
        RESULTS_DIR, f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
    params = gm.load_manifest(task).get(idx, {})
    with open(report_path, "w") as f:
        f.write(f"COMPARAISON APPARIEE — tache {task.upper()} / mission-{idx:02d} "
                f"— BRAS TEMOIN (protocole d'origine)\n")
        f.write("=" * 80 + "\n\nPARAMETRES DE LA MISSION\n" + "-" * 80 + "\n")
        for k in ("edges", "boxes", "length", "inner_radius", "radius_published",
                  "robots", "density", "distrib_ratio", "distrib_radius",
                  "marge_mur", "rab"):
            if k in params:
                f.write(f"  {k:17s} : {params[k]}\n")
        f.write(f"\nBudget par branche : {BUDGET_TOTAL} | firstTest : {FIRST_TEST} "
                f"| rounds : {N_ROUNDS}\n")
        f.write(f"Pool de depart     : {n_seeds_generated} FSMs aleatoires\n")
        f.write("Pre-filtrage simule: True (3 graines, NON decomptees du budget "
                "— comportement d'origine)\n")
        f.write("\n" + "=" * 80 + "\nSCORES D'ENTRAINEMENT FINAUX\n" + "-" * 80 + "\n")
        f.write(f"A) irace pur : {score_a_final:.2f}\n")
        f.write(f"B) LGEA LLM  : {score_b_final:.2f}   (origine : non-suivi)\n\n")
        f.write(f"Meilleure FSM (branche A) :\n{cli_a}\n\n")
        f.write(f"Meilleure FSM (branche B) :\n{cli_b}\n\n")
        f.write("=" * 80 + "\n")
        f.write("HISTORIQUE — BRANCHE A (irace pur, une ligne par iteration)\n")
        f.write("-" * 80 + "\n")
        f.write(format_history_a(parse_irace_iterations(log_a)))
        f.write("\n" + "=" * 80 + "\n")
        f.write("HISTORIQUE — BRANCHE B (LGEA LLM, une ligne par round)\n")
        f.write("-" * 80 + "\n")
        f.write("\n".join(round_history_log) + "\n")
    print(f"\n[SAVE] {report_path}")

    return dict(mission=f"mission-{idx:02d}", score_a=score_a_final,
                score_b=score_b_final)


def parse_missions_arg(spec: str) -> list:
    out = set()
    for part in spec.split(","):
        part = part.strip()
        if "-" in part:
            a, b = part.split("-")
            out.update(range(int(a), int(b) + 1))
        elif part:
            out.add(int(part))
    return sorted(i for i in out if 1 <= i <= gm.N_MISSIONS)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("task", choices=TASKS)
    ap.add_argument("--missions", default=f"1-{gm.N_MISSIONS}")
    ap.add_argument("--redo", action="store_true")
    ap.add_argument("--status", action="store_true")
    args = ap.parse_args()
    task = args.task

    wanted = parse_missions_arg(args.missions)

    if args.status:
        print("=" * 70)
        print(f" AVANCEMENT — {task.upper()}  (bras temoin)")
        print("=" * 70)
        done = 0
        for i in range(1, gm.N_MISSIONS + 1):
            rep = os.path.join(RESULTS_DIR,
                               f"PAIRED_COMPARISON_{task}_mission-{i:02d}.txt")
            if os.path.isfile(rep):
                done += 1
                print(f"  mission-{i:02d}  terminee")
            elif os.path.isdir(os.path.join(RUNS_DIR, task, f"mission-{i:02d}")):
                print(f"  mission-{i:02d}  interrompue (relance-la, elle repart de zero)")
        print("-" * 70)
        print(f"  {done}/{gm.N_MISSIONS} terminees")
        return

    check_base()
    check_api_key()

    if not os.path.isdir(os.path.join(MISSIONS_DIR, task)):
        raise SystemExit(
            f"[ERREUR] banc absent : {os.path.join(MISSIONS_DIR, task)}\n"
            f"         Le bras temoin doit tourner sur LES MEMES instances que le\n"
            f"         bras courant. Lance d'abord :\n"
            f"           python3 generate_missions.py {task}")

    irace_bin = get_irace_bin()
    _, irace_params, param_meta = load_grammar(task)
    system_prompt, _, _ = build_prompt_context(task, param_meta)
    system_prompt += MULTI_FSM_ADDENDUM
    client = anthropic.Anthropic(api_key=ANTHROPIC_API_KEY)
    r_script = f"{LLM_DIR}/extract_iteration_results.R"

    os.makedirs(RESULTS_DIR, exist_ok=True)
    with open(os.path.join(RESULTS_DIR, f"system_prompt_{task}.txt"), "w") as f:
        f.write(system_prompt)

    print("=" * 70)
    print(f" BRAS TEMOIN — {task.upper()} | {len(wanted)} mission(s)")
    print(f" protocole d'origine, memes instances que le bras courant")
    print(f" sorties : {RESULTS_DIR}")
    print("=" * 70)

    rows = []
    for idx in wanted:
        rep = os.path.join(RESULTS_DIR,
                           f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
        if os.path.isfile(rep) and not args.redo:
            print(f"\n[SKIP] mission-{idx:02d} : rapport deja present.")
            continue
        res = run_one_mission(task, idx, irace_bin, irace_params, param_meta,
                              system_prompt, client, r_script)
        if res:
            rows.append(res)

    if rows:
        print(f"\n{len(rows)} mission(s) terminee(s). "
              f"Comparaison avec le bras courant :")
        print(f"  python3 plot_comparaison.py {task} --origin")


if __name__ == "__main__":
    main()

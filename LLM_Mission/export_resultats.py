#!/usr/bin/env python3
"""
export_resultats.py — remplit resultats/ a partir de ce qui est deja dans runs/.

Ne relance AUCUNE simulation. Pour chaque mission deja traitee, ecrit :

    PAIRED_COMPARISON_<tache>_mission-NN.txt   le rapport, avec les deux
                                               historiques resumes cote a cote
    <TACHE>_mission-NN_irace.txt               journal complet de la branche A
    <TACHE>_mission-NN_LLM.txt                 journal complet de la branche B,
                                               round par round, avec ce que le
                                               LLM a propose a chaque fois

Usage :
    python3 export_resultats.py                # les trois taches
    python3 export_resultats.py aggregation
"""

import os
import re
import sys

from automode_llm_common_missions import RESULTS_DIR, LLM_DIR, TASKS
from run_paired_comparison_d_missions import (parse_irace_iterations, format_history_a,
                                     export_mission_logs, config_key,
                                     branche_a_terminee, N_ROUNDS)
import generate_missions as gm

HEADER_A = "HISTORIQUE — BRANCHE A (irace pur, une ligne par iteration)"
HEADER_B = "HISTORIQUE — BRANCHE B (LGEA LLM, une ligne par round)"
OLD_HEADER_B = "HISTORIQUE DE LA BRANCHE B"


def enrich_report(task: str, idx: int) -> str:
    """Ajoute le resume par iteration de la branche A au rapport existant."""
    path = os.path.join(RESULTS_DIR,
                        f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
    if not os.path.isfile(path):
        return None
    txt = open(path).read()
    if HEADER_A in txt:
        return "rapport deja a jour"

    log_a = os.path.join(LLM_DIR, "runs", task, f"mission-{idx:02d}", "irace_A.log")
    rows = parse_irace_iterations(log_a)
    if not rows:
        return "rapport inchange (irace_A.log absent)"

    split = txt.find("=" * 80 + "\n" + OLD_HEADER_B)
    if split < 0:
        split = txt.find("=" * 80 + "\n" + HEADER_B)
    if split < 0:
        return "rapport inchange (section branche B introuvable)"

    head, tail = txt[:split], txt[split:].replace(OLD_HEADER_B, HEADER_B, 1)
    with open(path, "w") as f:
        f.write(head + "=" * 80 + "\n" + HEADER_A + "\n" + "-" * 80 + "\n"
                + format_history_a(rows) + "\n" + tail)
    return f"rapport enrichi ({len(rows)} iterations)"


def _tsv(path):
    L = [l.rstrip("\n").split("\t") for l in open(path) if l.strip()]
    return L[0], L[1:]


def provenance_reelle(task: str, idx: int) -> dict:
    """{round: 'LLM'|'irace'} recalcule depuis les fichiers de la mission.

    Compare la configuration gagnante du round au pool fourni a irace, dans
    l'espace des parametres. L'ancien marquage comparait des chaines CLI et
    etiquetait a tort des FSM du LLM comme issues d'un tirage d'irace.
    """
    d = os.path.join(LLM_DIR, "runs", task, f"mission-{idx:02d}")
    ex = os.path.join(d, "execution-folder")
    out = {}
    for r in range(2, N_ROUNDS + 1):
        tsv = os.path.join(d, f"results_B{r}.tsv")
        pool = os.path.join(ex, f"llm_pool_B{r}.txt")
        if not (os.path.isfile(tsv) and os.path.isfile(pool)):
            continue
        h1, rows = _tsv(tsv)
        h2, prows = _tsv(pool)
        cols = [c for c in h2 if c in h1]
        best = min(rows, key=lambda x: float(dict(zip(h1, x)).get("MeanCost", "inf") or "inf"))
        kb = config_key(dict(zip(h1, best)), cols)
        out[r] = "LLM" if kb in {config_key(dict(zip(h2, p)), cols) for p in prows} else "irace"
    return out


def corriger_provenance(task: str, idx: int) -> str:
    path = os.path.join(RESULTS_DIR,
                        f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
    if not os.path.isfile(path):
        return None
    prov = provenance_reelle(task, idx)
    if not prov:
        return None
    txt = open(path).read()
    n = 0

    def repl(m):
        nonlocal n
        r = int(m.group(1))
        vrai = prov.get(r)
        if vrai is None or vrai == m.group(2).strip():
            return m.group(0)
        n += 1
        return f"round {m.group(1)} | best={m.group(3)} | origine={vrai:12s}"

    txt2 = re.sub(r"round\s+(\d+)\s*\|\s*best=(\s*[-\d.]+)\s*\|\s*origine=(\S+)\s*",
                  lambda m: (lambda r, sc, o: (f"round {m.group(1)} | best={sc} | origine="
                                               f"{prov.get(r, o):12s}"))(int(m.group(1)), m.group(2), m.group(3)),
                  txt)
    # compte les corrections
    avant = re.findall(r"round\s+(\d+)\s*\|[^|]*\|\s*origine=(\S+)", txt)
    apres = re.findall(r"round\s+(\d+)\s*\|[^|]*\|\s*origine=(\S+)", txt2)
    n = sum(1 for a, b in zip(avant, apres) if a != b)
    if n:
        open(path, "w").write(txt2)
        return f"provenance corrigee sur {n} round(s)"
    return "provenance deja correcte"


def main():
    wanted = [a for a in sys.argv[1:] if not a.startswith("-")] or TASKS
    os.makedirs(RESULTS_DIR, exist_ok=True)

    for task in wanted:
        runs = os.path.join(LLM_DIR, "runs", task)
        if not os.path.isdir(runs):
            continue
        idxs = sorted(int(m.group(1)) for m in
                      (re.fullmatch(r"mission-(\d+)", d) for d in os.listdir(runs)) if m)
        if not idxs:
            continue
        print(f"\n=== {task} — {len(idxs)} mission(s) dans runs/ ===")
        for i in idxs:
            notes = []
            r = enrich_report(task, i)
            if r:
                notes.append(r)
            pv = corriger_provenance(task, i)
            if pv:
                notes.append(pv)
            log_a = os.path.join(LLM_DIR, "runs", task, f"mission-{i:02d}", "irace_A.log")
            if os.path.isfile(log_a) and not branche_a_terminee(log_a):
                nb = len(parse_irace_iterations(log_a))
                notes.append(f"*** BRANCHE A INCOMPLETE : {nb}/{N_ROUNDS} iterations ***")
            files = export_mission_logs(task, i)
            notes += [os.path.basename(f) for f in files]
            print(f"  mission-{i:02d} : " + (", ".join(notes) if notes
                                             else "rien a exporter"))
    print(f"\nTout est dans {RESULTS_DIR}")


if __name__ == "__main__":
    main()

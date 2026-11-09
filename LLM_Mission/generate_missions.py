#!/usr/bin/env python3
"""
generate_missions.py

Genere les 20 missions a parametres variables pour une tache donnee, a partir de
l'instance publiee du depot de base.

Appele automatiquement par run_paired_comparison_d_missions.py au premier lancement ;
peut aussi etre lance seul pour inspecter le tirage avant de demarrer :

    python3 generate_missions.py aggregation
    python3 generate_missions.py                 # les trois taches

Chaque mission est un dossier autonome contenant une seule instance ARGoS, ce
qui permet de la passer telle quelle a irace via trainInstancesDir :

    missions/<tache>/mission-01/sequence-1.argos
    ...
    missions/<tache>/mission-20/sequence-1.argos
    missions/<tache>/manifest.tsv

Ce qui varie
------------
    number_robots               10 .. 30      (publie : 20)
    lenght_boxes                plage propre a la tache (ARENA_SPEC)
    dispersion initiale         0.35*R .. 1.00*R, avec plancher anti-collision
    portee range-and-bearing    0.30 .. 0.80 m   (publie : 0.50)

Ce qui ne varie pas : number_edges et number_boxes_per_edge, geles par tache.
Voir le bloc d'explication dans automode_llm_common_missions.py.
"""

import math
import os
import random
import re
import sys
import xml.etree.ElementTree as ET

from automode_llm_common_missions import (
    ARENA_SPEC, ROBOTS_RANGE, DISTRIB_RATIO_RANGE, RAB_RANGE,
    MISSIONS_DIR, TASKS, inner_radius, reference_argos, check_base, evaluate_fsm,
)

N_MISSIONS = 20
BENCH_SEED = 20260914          # graine maitresse : le tirage est reproductible

EPUCK_RADIUS = 0.035
MAX_PACKING = 0.22    # fraction d'occupation max du disque de depart

COLUMNS = ["mission", "task", "edges", "boxes", "length", "inner_radius",
           "radius_published", "robots", "density", "distrib_ratio",
           "distrib_radius", "marge_mur", "rab"]


def draw(rng: random.Random, task: str) -> dict:
    spec = ARENA_SPEC[task]
    edges, boxes = spec["edges"], spec["boxes"]
    lo, hi = spec["length_range"]

    length = round(rng.uniform(lo, hi), 4)
    r = inner_radius(edges, boxes, length)

    robots = rng.randint(*ROBOTS_RANGE)
    ratio = round(rng.uniform(*DISTRIB_RATIO_RANGE), 3)

    # Plancher anti-tassement : les robots occupent un disque de rayon
    # 1.5 * ratio * R (le terme c*rayon/2 de GetRandomPosition ajoute la moitie
    # du rayon). On exige que la fraction occupee reste sous MAX_PACKING.
    ratio_min = math.sqrt(robots / MAX_PACKING) * EPUCK_RADIUS / (1.5 * r)
    ratio = round(max(ratio, ratio_min), 3)
    ratio = min(ratio, DISTRIB_RATIO_RANGE[1])

    rab = round(rng.uniform(*RAB_RANGE), 3)
    distrib_radius = round(1.5 * ratio * r, 3)

    return dict(task=task, edges=edges, boxes=boxes, length=length,
                inner_radius=round(r, 4),
                radius_published=round(inner_radius(edges, boxes,
                                                    spec["length_published"]), 4),
                robots=robots,
                density=round(robots / (math.pi * r ** 2), 2),
                distrib_ratio=ratio, distrib_radius=distrib_radius,
                marge_mur=round(r - distrib_radius, 3),
                rab=rab)


def render(template: str, p: dict) -> str:
    t = template
    t = re.sub(r'number_robots="\d+"', f'number_robots="{p["robots"]}"', t)
    t = re.sub(r'number_edges="\d+"', f'number_edges="{p["edges"]}"', t)
    t = re.sub(r'number_boxes_per_edge="\d+"',
               f'number_boxes_per_edge="{p["boxes"]}"', t)
    t = re.sub(r'lenght_boxes="[\d.]+"', f'lenght_boxes="{p["length"]}"', t)

    # Le rayon de distribution, lu par le C++ patche. On l'insere juste apres
    # lenght_boxes, ou on le remplace s'il est deja la.
    if 'distribution_radius_ratio=' in t:
        t = re.sub(r'distribution_radius_ratio="[\d.]+"',
                   f'distribution_radius_ratio="{p["distrib_ratio"]}"', t)
    else:
        t = re.sub(r'(lenght_boxes="[\d.]+")',
                   rf'\1\n                distribution_radius_ratio="{p["distrib_ratio"]}"', t)

    # Le bloc <distribute> n'est PAS touche : MoveRobots() replace de toute
    # facon chaque robot au demarrage, il serait trompeur de le faire varier.
    t = re.sub(r'<entity\s+quantity="\d+"\s+max_trials="\d+"',
               f'<entity quantity="{p["robots"]}" max_trials="1000"', t)
    t = re.sub(r'(<epuck_range_and_bearing[^>]*?)range="[\d.]+"',
               rf'\1range="{p["rab"]}"', t)
    return t


def verify(path: str, p: dict):
    """Relit le fichier ecrit : XML valide, substitutions posees, bibliotheques presentes."""
    root = ET.parse(path).getroot()

    # Les .argos portent des chemins absolus vers libautomode.so et la loop
    # function. Si le gabarit est perime (depot deplace, setup_base.sh pas
    # relance), les 20 instances heriteraient de chemins morts et irace
    # renverrait 1e10 partout sans rien dire. On refuse de generer dans ce cas.
    for el in root.iter():
        lib = el.get("library")
        if lib and not os.path.isfile(lib):
            raise SystemExit(
                f"[ERREUR] {path} reference une bibliotheque introuvable :\n"
                f"           {lib}\n"
                f"         Le gabarit {reference_argos(p['task'])}\n"
                f"         porte des chemins perimes. Lance ./setup_base.sh, puis\n"
                f"         supprime le dossier missions/ et relance.")

    par = root.find("./loop_functions/params")
    ent = root.find(".//distribute/entity")
    rab = root.find(".//actuators/epuck_range_and_bearing")
    bad = []
    if int(par.get("number_robots")) != p["robots"]:
        bad.append("number_robots")
    if int(ent.get("quantity")) != p["robots"]:
        bad.append("quantity")
    if int(par.get("number_edges")) != p["edges"]:
        bad.append("number_edges")
    if int(par.get("number_boxes_per_edge")) != p["boxes"]:
        bad.append("number_boxes_per_edge")
    if abs(float(par.get("lenght_boxes")) - p["length"]) > 1e-9:
        bad.append("lenght_boxes")
    if abs(float(par.get("distribution_radius_ratio")) - p["distrib_ratio"]) > 1e-9:
        bad.append("distribution_radius_ratio")
    if rab is not None and abs(float(rab.get("range")) - p["rab"]) > 1e-9:
        bad.append("rab range")
    if bad:
        raise SystemExit(f"[ERREUR] substitution ratee dans {path} : {bad}")


def manifest_path(task: str) -> str:
    return os.path.join(MISSIONS_DIR, task, "manifest.tsv")


def mission_dir(task: str, idx: int) -> str:
    return os.path.join(MISSIONS_DIR, task, f"mission-{idx:02d}")


def mission_argos(task: str, idx: int) -> str:
    return os.path.join(mission_dir(task, idx), "sequence-1.argos")


def load_manifest(task: str) -> dict:
    """Retourne {index: parametres} depuis le manifeste deja ecrit."""
    p = manifest_path(task)
    out = {}
    if not os.path.isfile(p):
        return out
    with open(p) as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            if not line.strip():
                continue
            row = dict(zip(header, line.rstrip("\n").split("\t")))
            idx = int(row["mission"].split("-")[-1])
            out[idx] = dict(
                task=row["task"], edges=int(row["edges"]), boxes=int(row["boxes"]),
                length=float(row["length"]), inner_radius=float(row["inner_radius"]),
                radius_published=float(row["radius_published"]),
                robots=int(row["robots"]), density=float(row["density"]),
                distrib_ratio=float(row["distrib_ratio"]),
                distrib_radius=float(row["distrib_radius"]),
                marge_mur=float(row["marge_mur"]), rab=float(row["rab"]))
    return out


def generate(task: str, force: bool = False) -> dict:
    """Genere les 20 missions si besoin. Retourne {index: parametres}."""
    if not force:
        existing = load_manifest(task)
        if len(existing) == N_MISSIONS and all(
                os.path.isfile(mission_argos(task, i)) for i in existing):
            return existing

    template = open(reference_argos(task)).read()
    rng = random.Random(f"{BENCH_SEED}:{task}")

    rows, params = [], {}
    for i in range(1, N_MISSIONS + 1):
        p = draw(rng, task)
        os.makedirs(mission_dir(task, i), exist_ok=True)
        path = mission_argos(task, i)
        with open(path, "w") as f:
            f.write(render(template, p))
        verify(path, p)
        params[i] = p
        rows.append([f"mission-{i:02d}"] + [p[c] for c in COLUMNS[1:]])

    os.makedirs(os.path.join(MISSIONS_DIR, task), exist_ok=True)
    with open(manifest_path(task), "w") as f:
        f.write("\t".join(COLUMNS) + "\n")
        for r in rows:
            f.write("\t".join(str(x) for x in r) + "\n")

    return params


def print_table(task: str, params: dict):
    w = [11, 12, 5, 5, 7, 12, 8, 6, 7, 13, 13, 9, 5]
    print(f"\n=== {task} — {len(params)} missions (graine {BENCH_SEED}) ===")
    print("  ".join(c.ljust(x) for c, x in zip(COLUMNS, w)))
    for i in sorted(params):
        p = params[i]
        row = [f"mission-{i:02d}"] + [p[c] for c in COLUMNS[1:]]
        print("  ".join(str(v).ljust(x) for v, x in zip(row, w)))
    print(f"-> {os.path.join(MISSIONS_DIR, task)}")


def smoke_test(task: str, params: dict) -> int:
    """Execute une FSM triviale sur chaque instance generee.

    Une geometrie d'arene invalide ne se voit pas a la lecture du XML : elle
    fait planter ARGoS au premier pas de temps (LED hors de la grille). Mieux
    vaut le decouvrir ici, en trois minutes, qu'au milieu de la campagne.
    """
    print(f"   verification par execution ({len(params)} instances)...", end="", flush=True)
    bad = 0
    for i in sorted(params):
        # FSM qui BOUGE : l'ancienne (--s0 1, tout le monde a l'arret) ne
        # provoquait aucune collision, donc elle ne pouvait pas detecter une
        # arene surpeuplee. Exploration en marche aleatoire les fait se
        # bousculer, ce qui est exactement le cas qui faisait planter ARGoS.
        score = evaluate_fsm("--nstates 1 --s0 0 --rwm0 50 --cle0 0",
                             mission_argos(task, i), seeds=[1234, 42])
        if score is None:
            if bad == 0:
                print()
            print(f"   [ECHEC] mission-{i:02d} : ARGoS ne rend aucun score "
                  f"(R={params[i]['inner_radius']:.3f}, {params[i]['robots']} robots)")
            bad += 1
    if bad == 0:
        print(" toutes valides.")
    else:
        print(f"   {bad} instance(s) invalide(s) — NE LANCE PAS la campagne.")
    return bad


def main():
    check_base()
    wanted = sys.argv[1:] or TASKS
    verify_run = "--no-verify" not in sys.argv
    wanted = [w for w in wanted if not w.startswith("--")] or TASKS
    for task in wanted:
        if task not in ARENA_SPEC:
            raise SystemExit(f"[ERREUR] tache inconnue : {task} "
                             f"(attendu : {', '.join(TASKS)})")
        params = generate(task, force=True)
        print_table(task, params)
        if verify_run:
            smoke_test(task, params)


if __name__ == "__main__":
    main()
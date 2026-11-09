#!/usr/bin/env python3
"""
patch_distribution_radius.py — rend le rayon de distribution des robots
proportionnel a la taille de l'arene, au lieu d'etre fige a 0.4.

LE PROBLEME
-----------
SeqLoopFunction::GetRandomPosition() contient :

    m_fDistributionRadius = 0.4;            // en dur
    fPosX = (c * m_fDistributionRadius / 2) + m_fDistributionRadius * cos(...)
    fPosY = (d * m_fDistributionRadius / 2) + m_fDistributionRadius * sin(...)

Cette ligne ECRASE la valeur que Init() venait de calculer
(m_fDistributionRadius = GetArenaRadious()). Deux consequences :

  1. le bloc <distribute> du .argos est sans effet, les robots sont toujours
     replaces par MoveRobots() dans un disque de rayon <= ~0.68 ;
  2. plus l'arene est petite, plus ce paquet de robots demarre pres du mur.
     A R = 0.733 il ne reste que 0.05 m : le moteur 2D finit par pousser un
     robot a travers la paroi et ARGoS leve
         CGrid<ENTITY>::GetEntitiesAt() : Position <...> out of bounds

LE CORRECTIF
------------
Le rayon devient une FRACTION du rayon interne de l'arene, lue dans le .argos :

    <params ... distribution_radius_ratio="0.45" />
    m_fDistributionRadius = m_fDistributionRatio * GetArenaRadious();

Par defaut 0.45, ce qui reproduit presque exactement le comportement publie
aux tailles d'arene publiees (extension maximale 0.585 pour aggregation,
0.611 pour stop, contre 0.6 avec la valeur figee). Les petites arenes
deviennent sures, et la dispersion initiale devient un parametre reellement
variable.

A DECLARER DANS LE MEMOIRE : c'est une modification du code publie.

Usage :
    python3 patch_distribution_radius.py            # applique
    python3 patch_distribution_radius.py --check    # verifie sans ecrire
"""

import os
import re
import shutil
import sys

MARKER = "LLM_MISSION_DISTRIBUTION_RATIO"


def find_root(start: str):
    d = start
    while d != "/":
        if os.path.isdir(os.path.join(d, "software", "argos3-installation")):
            return d
        d = os.path.dirname(d)
    return None


def patch_header(path: str, check: bool) -> str:
    src = open(path).read()
    if MARKER in src:
        return "deja patche"
    anchor = "    Real m_fLenghtBoxes;"
    if anchor not in src:
        return "[ECHEC] ancre introuvable dans le .h"
    new = (anchor + "\n"
           f"    Real m_fDistributionRatio;   // {MARKER}\n")
    if not check:
        shutil.copy2(path, path + ".orig")
        open(path, "w").write(src.replace(anchor, new, 1))
    return "patche"


def patch_source(path: str, check: bool) -> str:
    src = open(path).read()
    if MARKER in src:
        return "deja patche"

    anchor = ('        GetNodeAttributeOrDefault(cParametersNode, "lenght_boxes",'
              '         m_fLenghtBoxes,      (Real)   0.25);')
    if anchor not in src:
        return "[ECHEC] ancre 'lenght_boxes' introuvable dans le .cpp"
    lecture = (anchor + "\n"
               '        // ' + MARKER + " : fraction du rayon interne de l'arene\n"
               '        GetNodeAttributeOrDefault(cParametersNode, "distribution_radius_ratio",'
               ' m_fDistributionRatio, (Real) 0.45);')
    src = src.replace(anchor, lecture, 1)

    old = "    m_fDistributionRadius = 0.4;"
    if old not in src:
        return "[ECHEC] la ligne 'm_fDistributionRadius = 0.4;' est introuvable"
    new = ("    // " + MARKER + " : le rayon suit la taille de l'arene au lieu\n"
           "    // d'etre fige a 0.4, sinon les petites arenes font demarrer les\n"
           "    // robots colles au mur et le moteur 2D en ejecte un.\n"
           "    m_fDistributionRadius = m_fDistributionRatio * GetArenaRadious();")
    src = src.replace(old, new, 1)

    if not check:
        shutil.copy2(path, path + ".orig")
        open(path, "w").write(src)
    return "patche"


def main():
    check = "--check" in sys.argv
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.environ.get("MANDARINA_ROOT") or find_root(here)
    if not root:
        raise SystemExit("[ERREUR] racine du depot introuvable. "
                         "Definis MANDARINA_ROOT.")

    base = os.path.join(root, "software", "argos3-installation",
                        "experiments-loop-functions", "loop-functions", "moca")
    h = os.path.join(base, "SeqLoopFunc.h")
    c = os.path.join(base, "SeqLoopFunc.cpp")
    for p in (h, c):
        if not os.path.isfile(p):
            raise SystemExit(f"[ERREUR] introuvable : {p}")

    print(f"Depot : {root}")
    print(f"  SeqLoopFunc.h   : {patch_header(h, check)}")
    print(f"  SeqLoopFunc.cpp : {patch_source(c, check)}")

    if check:
        print("\n(--check : rien n'a ete ecrit)")
        return

    print("\nLes fichiers d'origine sont conserves en .orig")
    print("\nIL FAUT MAINTENANT RECOMPILER les loop functions :")
    print(f"  cd {root}/software/argos3-installation/experiments-loop-functions/build")
    print("  make -j$(nproc)")
    print("\nPuis regenerer les instances :")
    print("  rm -rf missions/")
    print("  python3 generate_missions.py")


if __name__ == "__main__":
    main()

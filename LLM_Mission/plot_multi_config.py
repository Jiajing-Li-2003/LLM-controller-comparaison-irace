#!/usr/bin/env python3
"""
plot_multi_config.py — toutes les configurations de prompt sur une figure.

Lit les rapports MULTI_<tache>_rep-NN.txt produits par
run_comparaison_multi_config.py et ecrit dans resultats_multi/figures/ :

    <tache>_comparaison_multi[_brut].png   une courbe par configuration,
                                           plus la courbe irace de reference
    <tache>_difference_multi[_brut].png    difference appariee de chaque
                                           configuration avec irace
    <tache>_donnees_multi[_brut].tsv       les nombres

Usage :
    python3 plot_multi_config.py aggregation
    python3 plot_multi_config.py aggregation --brut
    python3 plot_multi_config.py aggregation --configs complet,sans_classement
    python3 plot_multi_config.py aggregation --barres
    python3 plot_multi_config.py aggregation --legende "upper right,lower right"
    python3 plot_multi_config.py --lister            # les noms de configuration

Choisir les courbes
-------------------
--configs <noms separes par des virgules> ne trace que ces configurations.
--sans-irace retire la courbe de reference. Les noms exacts sont donnes par
--lister, et le nom du fichier porte le nombre de configurations tracees, donc
une figure partielle n'ecrase pas la figure complete.

ATTENTION : --configs ne cache pas des courbes, il restreint l'analyse. Une
repetition n'est retenue que si elle porte TOUTES les configurations
demandees, donc en demander moins peut faire ENTRER des repetitions
supplementaires et changer le n. Le script affiche toujours le n retenu.

Ou poser la legende
-------------------
--legende <position> : "upper right" par defaut. Deux positions separees
par une virgule reglent separement la figure de comparaison et celle des
ecarts, dont les zones vides ne sont pas aux memes endroits :
    --legende "upper right,lower right"
Positions : "upper left", "upper right",
"lower left", "lower right", "center left", "center right", "best", ou
"dessous" pour la remettre sous le cadre. A l'interieur, elle rend au
graphique la hauteur qu'elle occupait dessous.

Forme des intervalles de confiance
----------------------------------
Par defaut chaque serie porte une BANDE pleine, comme irace. Au-dela de trois
ou quatre configurations les bandes se recouvrent et la figure devient dense :
--barres les remplace par des barres d'erreur legerement decalees en x, qui
restent distinctes meme quand les courbes se croisent. Les deux versions ont
des noms de fichier differents et ne s'ecrasent pas.

Les deux lectures
-----------------
Par defaut chaque courbe de configuration est tracee en MINIMUM COURANT : le
meilleur score connu depuis le round 1. Elle ne peut donc jamais remonter.
Avec --brut, elle montre le meilleur de CHAQUE round, qui lui peut remonter.
Une courbe brute qui remonte signale une recherche qui n'a pas converge : le
score affiche vient d'un tirage chanceux anterieur. Les deux lectures ont des
noms de fichier differents, elles ne s'ecrasent pas.

La courbe irace est la MEME pour toutes les configurations, par construction :
une seule branche A est produite par repetition et partagee. C'est ce qui rend
les configurations comparables entre elles et pas seulement a irace.
"""

import math
import os
import re
import sys

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    raise SystemExit("[ERREUR] matplotlib manquant.  pip install matplotlib")

from automode_multi_config import (RESULTS_DIR, CONFIGS, config_libelle,
                                   config_resume, TASKS)

FIG_DIR = os.path.join(RESULTS_DIR, "figures")

COL_IRACE = "#1F4E79"
# Teintes ET clartes differentes, pour rester lisible en noir et blanc.
COL_CONFIG = {
    "complet": "#C2551F",
    "sans_classement": "#7A4E9E",
    "sans_classement_ni_champion": "#2D7D6E",
    "sans_contrainte_ni_classement_ni_champion": "#A8324A",
    "sans_contrainte": "#8A6D1F",
    "sans_champion": "#1F6E8A",
    "temoin": "#5A5A5A",
}
# Les figures partent dans un article en anglais, mais les rapports et la
# sortie console restent en francais : la traduction vit donc ICI, et pas dans
# CONFIGS, pour ne pas changer ce qui est ecrit dans les rapports.
# Une configuration absente de cette table garde son libelle d'origine.
LIBELLES_EN = {
    "complet": "full prompt",
    "sans_classement": "no ranking (champion kept)",
    "sans_champion": "no champion (ranking kept)",
    "sans_classement_ni_champion": "no ranking, no champion",
    "sans_contrainte": "no connectivity constraint",
    "sans_contrainte_ni_classement": "no constraint, no ranking",
    "sans_contrainte_ni_champion": "no constraint, no champion",
    "sans_contrainte_ni_classement_ni_champion":
        "no constraint, no ranking, no champion",
    "temoin": "control (nothing removed)",
}


def libelle(nom: str) -> str:
    """Libelle anglais d'une configuration, a defaut celui du module."""
    return LIBELLES_EN.get(nom, config_libelle(nom))


MARQUEURS = ["o", "s", "^", "D", "v", "P", "X"]
COL_SECOURS = ["#8A6D1F", "#4A4A8A", "#1F6E4A", "#8A3A6D"]

T_975 = {1: 12.706, 2: 4.303, 3: 3.182, 4: 2.776, 5: 2.571, 6: 2.447, 7: 2.365,
         8: 2.306, 9: 2.262, 10: 2.228, 11: 2.201, 12: 2.179, 13: 2.160,
         14: 2.145, 15: 2.131, 16: 2.120, 17: 2.110, 18: 2.101, 19: 2.093,
         20: 2.086, 21: 2.080, 22: 2.074, 23: 2.069, 24: 2.064, 25: 2.060,
         26: 2.056, 27: 2.052, 28: 2.048, 29: 2.045, 30: 2.042}


def t_critical(df: int) -> float:
    if df <= 0:
        return float("nan")
    return T_975.get(df, 1.960 + 2.40 / df)


def mean_ci(values: list):
    n = len(values)
    if n == 0:
        return float("nan"), float("nan")
    m = sum(values) / n
    if n == 1:
        return m, float("nan")
    var = sum((v - m) ** 2 for v in values) / (n - 1)
    return m, t_critical(n - 1) * math.sqrt(var / n)


def running_min(xs: list) -> list:
    out, best = [], float("inf")
    for x in xs:
        best = min(best, x)
        out.append(best)
    return out


# ---------------------------------------------------------------------------
# Lecture des rapports
# ---------------------------------------------------------------------------
RE_ROUND = re.compile(r"^round\s+(\d+)\s*\|\s*best=\s*([-\d.]+)", re.M)
RE_ITER_A = re.compile(r"^\s*(\d+)\s*\|\s*([-\d.]+)\s*\|\s*\S+\s*\|\s*\d+\s*\|\s*\d+\s*$", re.M)


def lire_rapport(path: str):
    """Retourne (serie_irace, {config: serie_brute}) pour une repetition."""
    txt = open(path, errors="ignore").read()

    # La branche A occupe tout ce qui precede le premier bloc CONFIG.
    coupe = txt.find("\nCONFIG ")
    bloc_a = txt[:coupe] if coupe > 0 else txt
    serie_a = [float(v) for _, v in RE_ITER_A.findall(bloc_a)]

    configs = {}
    morceaux = re.split(r"^CONFIG\s+(\S+)\s*\|", txt, flags=re.M)
    for i in range(1, len(morceaux) - 1, 2):
        nom, corps = morceaux[i], morceaux[i + 1]
        serie = [float(v) for _, v in RE_ROUND.findall(corps)]
        if serie:
            configs[nom] = serie
    return serie_a, configs


def collecter(task: str, cumulatif: bool, voulues: list):
    """{config: [series par repetition]} plus la serie irace commune."""
    if not os.path.isdir(RESULTS_DIR):
        raise SystemExit(f"[ERREUR] {RESULTS_DIR} n'existe pas.\n"
                         f"         Lance d'abord : python3 "
                         f"run_comparaison_multi_config.py {task}")
    fichiers = sorted(f for f in os.listdir(RESULTS_DIR)
                      if re.fullmatch(rf"MULTI_{re.escape(task)}_rep-\d+\.txt", f))
    if not fichiers:
        raise SystemExit(f"[ERREUR] aucun rapport MULTI_{task}_rep-NN.txt dans "
                         f"{RESULTS_DIR}.\n"
                         f"         Une repetition n'ecrit son rapport qu'apres sa "
                         f"premiere configuration terminee.")

    irace, par_config = [], {}
    for f in fichiers:
        a, cfgs = lire_rapport(os.path.join(RESULTS_DIR, f))
        if not a or not cfgs:
            continue
        # Une repetition n'entre dans la moyenne que si elle porte TOUTES les
        # configurations demandees : sinon les courbes reposeraient sur des
        # sous-ensembles differents de repetitions et ne seraient plus appariees.
        if any(c not in cfgs for c in voulues):
            continue
        irace.append(a)
        for c in voulues:
            serie = running_min(cfgs[c]) if cumulatif else cfgs[c]
            par_config.setdefault(c, []).append(serie)
    return irace, par_config, len(fichiers)


def tronquer(series: list) -> int:
    return min((len(s) for s in series), default=0)


def moyennes(series: list, n: int):
    return [mean_ci([s[r] for s in series]) for r in range(n)]


# ---------------------------------------------------------------------------
# Figures
# ---------------------------------------------------------------------------
def couleur(nom: str, i: int) -> str:
    return COL_CONFIG.get(nom, COL_SECOURS[i % len(COL_SECOURS)])


POSITIONS = ("upper left", "upper right", "lower left", "lower right",
             "center left", "center right", "best", "dessous")


def legende(ax, fig, position="upper right"):
    """Legende DANS le cadre, par defaut en haut a gauche.

    La placer a l'interieur rend au graphique toute la hauteur qui servait a
    la loger dessous : les courbes y gagnent nettement en lisibilite, ce qui
    compte plus que les quelques points qu'elle peut recouvrir.

    Le cadre est opaque a 90 % : ce qui passe derriere reste devine sans
    rendre les libelles penibles a lire. Si elle tombe malgre tout sur une
    courbe importante — les missions n'ont pas toutes la meme allure —
    --legende <position> la deplace.
    """
    if position == "dessous":
        ax.legend(fontsize=9, loc="upper center", bbox_to_anchor=(0.5, -0.175),
                  ncol=2, frameon=False, handlelength=2.4, columnspacing=1.8)
        return
    ax.legend(fontsize=9, loc=position, frameon=True, framealpha=.90,
              facecolor="white", edgecolor="#c4c8ce", handlelength=2.2,
              borderpad=.6, labelspacing=.45).set_zorder(10)


def badge(fig, task, n_rep, cumulatif, reserve_pouces=0.62):
    mode = ("running minimum (best since round 1)" if cumulatif
            else "raw values (best of each round)")
    fig.subplots_adjust(top=fig.subplotpars.top - reserve_pouces / fig.get_figheight())
    fig.text(0.008, 0.992,
             f"shared base: one starting pool, one irace branch\n{mode}",
             ha="left", va="top", fontsize=8, color="#1a1a1a", linespacing=1.5,
             bbox=dict(boxstyle="round,pad=0.35", facecolor="#eceff3",
                       edgecolor="#9aa4b0", linewidth=0.6))


def fig_comparaison(task, irace, par_config, n, cumulatif, path,
                    barres=False, position="upper right", avec_irace=True):
    fig, ax = plt.subplots(figsize=(9, 5.9))
    rounds = list(range(1, n + 1))

    # Chaque serie porte son IC 95 %, mais pas sous la meme forme.
    #
    # irace garde une BANDE pleine : c'est la reference, une seule, et la bande
    # se lit d'un coup d'oeil. Les configurations recoivent des BARRES : avec
    # cinq ou six bandes superposees, les remplissages se melangent en une
    # bouillie ou plus aucune courbe n'est lisible. Les barres restent nettes
    # meme quand les courbes se croisent.
    #
    # Les barres sont DECALEES en x les unes par rapport aux autres, sinon
    # elles se recouvrent exactement la ou les courbes se rapprochent, c'est-
    # a-dire precisement la ou il faut pouvoir les distinguer.
    mi = moyennes(irace, n)
    m, e = zip(*mi)
    lo = [v - (d if d == d else 0) for v, d in zip(m, e)]
    hi = [v + (d if d == d else 0) for v, d in zip(m, e)]
    if avec_irace:
        ax.fill_between(rounds, lo, hi, color=COL_IRACE, alpha=.15,
                        linewidth=0, zorder=1)
        ax.plot(rounds, m, "-o", color=COL_IRACE, ms=4.5, lw=2.4,
                label="irace", zorder=5)

    k = len(par_config)
    largeur = 0.42                      # etalement total du decalage, en rounds
    for i, (nom, series) in enumerate(par_config.items()):
        mc, ec = zip(*moyennes(series, n))
        c = couleur(nom, i)
        bas = [v - (d if d == d else 0) for v, d in zip(mc, ec)]
        haut = [v + (d if d == d else 0) for v, d in zip(mc, ec)]
        if barres:
            dx = (i - (k - 1) / 2) * (largeur / max(k - 1, 1))
            ax.errorbar([r + dx for r in rounds], mc,
                        yerr=[(d if d == d else 0) for d in ec],
                        fmt="none", ecolor=c, elinewidth=1.0, capsize=2.5,
                        capthick=1.0, alpha=.85, zorder=3)
        else:
            # Bandes pleines, comme pour irace. Avec cinq ou six series elles
            # se recouvrent : l'opacite est volontairement basse et les
            # courbes passent au-dessus, pour qu'au moins les moyennes
            # restent lisibles sous les remplissages.
            ax.fill_between(rounds, bas, haut, color=c, alpha=.13,
                            linewidth=0, zorder=2)
        ax.plot(rounds, mc, "-", marker=MARQUEURS[i % len(MARQUEURS)],
                color=c, ms=4.5, lw=1.9,
                label=libelle(nom), zorder=4)

    ax.set_xlabel("round / iteration")
    ax.set_ylabel("mean score")
    ax.set_xticks(rounds)
    ax.grid(alpha=.3)
    legende(ax, fig, position)
    ax.set_title(f"{task.upper()} — prompt configurations compared\n"
                 f"n = {len(irace)} paired repetitions", fontsize=12)
    forme = ("band for irace, offset error bars for the configurations"
             if barres else "one band per series")
    fig.text(.5, .012,
             "Shared base: the configurations are paired with one another.  "
             f"Two-sided 95 % CI (t-test): {forme}.",
             ha="center", fontsize=7.5, color="#555555")
    fig.tight_layout(rect=(0, .055 if position != "dessous" else .11, 1, 1))
    badge(fig, task, len(irace), cumulatif)
    fig.savefig(path, dpi=150)
    plt.close(fig)


def fig_differences(task, irace, par_config, n, cumulatif, path,
                    position="upper right"):
    fig, ax = plt.subplots(figsize=(9, 5.9))
    rounds = list(range(1, n + 1))
    ax.axhline(0, color="black", lw=1, zorder=2)

    stats = {}
    for i, (nom, series) in enumerate(par_config.items()):
        diffs_par_round = [[series[k][r] - irace[k][r] for k in range(len(series))]
                           for r in range(n)]
        mm = [mean_ci(d) for d in diffs_par_round]
        m, e = zip(*mm)
        c = couleur(nom, i)
        ax.errorbar(rounds, m, yerr=[(d if d == d else 0) for d in e],
                    fmt="-" + MARQUEURS[i % len(MARQUEURS)], color=c, ms=4.5,
                    lw=1.9, capsize=3, label=libelle(nom), zorder=3)
        stats[nom] = mm

    ax.set_xlabel("round / iteration")
    ax.set_ylabel("paired difference:  configuration − irace")
    ax.set_xticks(rounds)
    ax.grid(alpha=.3)
    legende(ax, fig, position)
    ax.set_title(f"{task.upper()} — gap between each configuration and irace\n"
                 f"n = {len(irace)} paired repetitions, "
                 f"bars = two-sided 95 % CI (t-test)", fontsize=12)
    fig.text(.5, .012,
             "Below zero: the configuration does better than irace.  "
             "Shared base: one starting pool, one irace branch per repetition.",
             ha="center", fontsize=7.5, color="#555555")
    fig.tight_layout(rect=(0, .055 if position != "dessous" else .11, 1, 1))
    badge(fig, task, len(irace), cumulatif)
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return stats


def ecrire_tsv(path, task, irace, par_config, n, cumulatif, stats):
    mode = "minimum_courant" if cumulatif else "brut"
    with open(path, "w") as f:
        f.write(f"# tache : {task} | lecture : {mode} | "
                f"n = {len(irace)} repetitions appariees\n")
        f.write("# socle commun : un seul pool de depart et une seule branche irace\n")
        for nom in par_config:
            f.write(f"# config {nom} : {config_resume(nom)}\n")
        cols = ["round", "n_rep", "irace_moy", "irace_ic95"]
        for nom in par_config:
            # moyenne et IC propres a la configuration (ce que montrent les
            # barres de la figure), puis l'ecart appariee avec irace et son IC
            # (ce que montre la figure des differences).
            cols += [f"{nom}_moy", f"{nom}_ic95",
                     f"{nom}_diff_moy", f"{nom}_diff_ic95"]
        f.write("\t".join(cols) + "\n")
        mi = moyennes(irace, n)
        mc = {nom: moyennes(s, n) for nom, s in par_config.items()}
        for r in range(n):
            ligne = [str(r + 1), str(len(irace)),
                     f"{mi[r][0]:.3f}", f"{mi[r][1]:.3f}"]
            for nom in par_config:
                dm, de = stats[nom][r]
                cm, ce = mc[nom][r]
                ligne += [f"{cm:.3f}", f"{ce:.3f}", f"{dm:.3f}", f"{de:.3f}"]
            f.write("\t".join(ligne) + "\n")


# ---------------------------------------------------------------------------
def valeur_de(args, drapeau):
    """Valeur qui suit un drapeau, et les indices a ne pas lire comme taches."""
    for i, a in enumerate(args):
        c = a.replace("_", "-")
        if c == drapeau and i + 1 < len(args):
            return args[i + 1], {i, i + 1}
        if c.startswith(drapeau + "="):
            return a.split("=", 1)[1], {i}
    return None, set()


def lister_configs():
    print("\n  Configurations disponibles (--configs pour en choisir) :")
    for n in CONFIGS:
        print(f"    {n:44s} {config_libelle(n)}")
    print()


def main():
    args = sys.argv[1:]
    drapeaux = [a.replace("_", "-") for a in args]
    cumulatif = "--brut" not in drapeaux
    barres = "--barres" in drapeaux
    avec_irace = "--sans-irace" not in drapeaux

    if "--lister" in drapeaux or "--help" in drapeaux or "-h" in drapeaux:
        print(__doc__)
        lister_configs()
        return

    sautes = set()

    # --configs : quelles courbes tracer
    spec, idx = valeur_de(args, "--configs")
    sautes |= idx
    voulues = list(CONFIGS)
    if spec is not None:
        voulues = [c.strip() for c in spec.split(",") if c.strip()]
        inconnues = [c for c in voulues if c not in CONFIGS]
        if inconnues:
            print(f"[ERREUR] configuration(s) inconnue(s) : {', '.join(inconnues)}")
            lister_configs()
            raise SystemExit(1)
        if not voulues:
            raise SystemExit("[ERREUR] --configs ne designe aucune configuration.")

    # --legende : ou poser la legende
    # Une position, ou deux separees par une virgule : la comparaison et les
    # ecarts n'ont pas leurs zones vides au meme endroit, et une seule valeur
    # ne peut pas convenir aux deux.
    pos, idx = valeur_de(args, "--legende")
    sautes |= idx
    morceaux = [m.replace("_", " ").strip()
                for m in (pos or "upper right").split(",")]
    if len(morceaux) == 1:
        morceaux *= 2
    if len(morceaux) != 2:
        raise SystemExit("[ERREUR] --legende attend une position, ou deux "
                         "separees par une virgule.")
    for m in morceaux:
        if m not in POSITIONS:
            raise SystemExit(f"[ERREUR] --legende : position inconnue '{m}'.\n"
                             f"         Attendu : {', '.join(POSITIONS)}")
    position, position_diff = morceaux

    taches = [a for i, a in enumerate(args)
              if not a.startswith("-") and i not in sautes] or TASKS

    os.makedirs(FIG_DIR, exist_ok=True)
    suff = (("" if cumulatif else "_brut") + ("_barres" if barres else "")
            + ("" if len(voulues) == len(CONFIGS) else f"_{len(voulues)}cfg")
            + ("" if avec_irace else "_sansirace"))

    for task in taches:
        if task not in TASKS:
            raise SystemExit(f"[ERREUR] tache inconnue : {task} "
                             f"(attendu : {', '.join(TASKS)})")
        irace, par_config, n_fichiers = collecter(task, cumulatif, voulues)
        if not irace:
            print(f"[{task}] {n_fichiers} rapport(s) trouve(s), mais aucun ne porte "
                  f"les {len(voulues)} configurations demandees.")
            print(f"         Suis l'avancement avec :")
            print(f"           python3 run_comparaison_multi_config.py {task} --status")
            continue
        n = min(tronquer(irace), min(tronquer(s) for s in par_config.values()))
        if n < 1:
            print(f"[{task}] series vides, rien a tracer.")
            continue

        print(f"\n=== {task} — {len(irace)} repetition(s) completes, "
              f"{len(par_config)} configuration(s), {n} rounds ===")
        if len(voulues) != len(CONFIGS):
            print(f"  (selection : {', '.join(voulues)}"
                  f"{' — irace masque' if not avec_irace else ''})")

        p1 = os.path.join(FIG_DIR, f"{task}_comparaison_multi{suff}.png")
        fig_comparaison(task, irace, par_config, n, cumulatif, p1, barres,
                        position, avec_irace)
        print(f"  {p1}")

        p2 = os.path.join(FIG_DIR, f"{task}_difference_multi{suff}.png")
        stats = fig_differences(task, irace, par_config, n, cumulatif, p2,
                                position_diff)
        print(f"  {p2}")

        p3 = os.path.join(FIG_DIR, f"{task}_donnees_multi{suff}.tsv")
        ecrire_tsv(p3, task, irace, par_config, n, cumulatif, stats)
        print(f"  {p3}")

        print(f"\n  Au dernier round, ecart avec irace :")
        for nom in par_config:
            dm, de = stats[nom][n - 1]
            verdict = ("significativement meilleure" if dm + de < 0 else
                       "significativement moins bonne" if dm - de > 0 else
                       "pas d'ecart significatif")
            print(f"    {config_libelle(nom):44s} {dm:+9.1f} "
                  f"[{dm - de:+8.1f} ; {dm + de:+8.1f}]  {verdict}")


if __name__ == "__main__":
    main()
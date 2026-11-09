#!/usr/bin/env python3
"""
plot_comparaison.py — figures de comparaison irace vs LLM a partir de resultats/.

Lit les rapports PAIRED_COMPARISON_<tache>_mission-NN.txt (et, a defaut,
runs/<tache>/mission-NN/irace_A.log) et produit dans resultats/figures/ :

    <tache>_par_mission.png     une courbe A vs B par mission, en grille
    <tache>_moyenne.png         moyenne sur les missions, round par round,
                                avec intervalle de confiance bilateral a 95 %
    <tache>_normalise.png       meme chose apres normalisation par le score
                                du round 1 de chaque mission
    <tache>_difference.png      difference appariee B - A, avec son IC 95 %
    <tache>_donnees.tsv         les nombres, pour le memoire

Usage :
    python3 plot_comparaison.py                 # les trois taches
    python3 plot_comparaison.py aggregation
    python3 plot_comparaison.py aggregation --brut   # B sans minimum courant

DEUX LECTURES DE LA MEME CAMPAGNE
---------------------------------
Par defaut, la branche B est tracee en MINIMUM COURANT : le meilleur score
connu depuis le round 1. Par construction cette courbe ne remonte jamais.
Avec --brut, elle montre le meilleur de CHAQUE round, qui lui peut remonter.
Une courbe brute qui remonte alors que le minimum courant reste plat signale
une recherche qui n'a pas converge : le score affiche vient d'un tirage
chanceux anterieur. Les deux lectures portent desormais des noms de fichier
differents (suffixe _brut) et un badge different, donc elles ne s'ecrasent
plus.

CAMPAGNES ARCHIVEES
-------------------
    python3 plot_comparaison.py aggregation --archive campagne_01 --complet --brut

--archive <nom> prefixe les dossiers par archives/<nom>/ ; le drapeau de
campagne continue de choisir lequel lire a l'interieur. Les figures sont
ecrites dans l'archive, a cote des donnees dont elles sortent.

Chaque campagne a son drapeau, ses dossiers et son libelle, et ce libelle est
inscrit sur chaque figure produite :

    (aucun)                     resultats/                "prompt complet — banc de missions"
    --origin                    resultats_origin/         "bras temoin — protocole d'origine"
    --complet                   resultats_complet/        "instance publiee — classement complet"
    --sans-contrainte-ni-info   resultats_sans_..._info/  "ablation — sans contrainte ni classement"
    --sans-contrainte           resultats_sans_contrainte/ "ablation — sans contrainte de connectivite"
    --sans-info                 resultats_sans_info/      "ablation — sans information de classement"

Les tirets bas sont acceptes. Un drapeau inconnu arrete le script au lieu de
retomber silencieusement sur la campagne par defaut.

Sur les deux definitions de "meilleur score"
--------------------------------------------
La branche A rapporte, a chaque iteration, la moyenne de son elite reevaluee :
cette valeur peut remonter d'une iteration a l'autre. La branche B rapporte le
meilleur MeanCost observe dans le round. Par defaut le script applique un
MINIMUM COURANT a la branche B, pour obtenir la meme semantique de
"meilleur connu apres r rounds". Les deux estimateurs restent de nature
differente : celui d'irace est moyenne sur beaucoup plus d'evaluations, et le
minimum d'estimations bruitees est optimiste. C'est rappele sur les figures.
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

from automode_llm_common_missions import RESULTS_DIR, LLM_DIR, TASKS
from run_paired_comparison_d_missions import parse_irace_iterations, mission_status

# Chaque campagne ecrit dans sa propre paire de dossiers. Un seul endroit a
# modifier pour en brancher une nouvelle : son drapeau, ses deux dossiers et
# le script qui la produit. Toutes ecrivent le meme format de rapport, donc
# les memes figures et le meme TSV sont produits, chacun chez soi.
_ABLATION = "run_paired_comparaison_sans_contrainte_ni_info.py"
_MISSIONS = "run_paired_comparison_d_missions.py"

# drapeau : (dossier resultats, dossier runs, script, libelle porte par les figures)
#
# Le libelle est ce qui distingue une figure d'une autre une fois les fichiers
# sortis de leur dossier : toutes les campagnes produisent un
# aggregation_moyenne.png, et rien ne les differencie une fois cote a cote dans
# un article. Il est donc inscrit sur chaque figure et en tete du TSV.
CAMPAGNES = {
    "--origin": ("resultats_origin", "runs_origin",
                 "run_paired_comparison_d_origin.py",
                 "bras temoin — protocole d'origine"),
    "--complet": ("resultats_complet", "runs_complet",
                  "run_paired_comparaison_complet.py",
                  "instance publiee — classement complet"),
    # Une etiquette par interrupteur : contrainte, classement, champion.
    # Toutes les combinaisons utiles ont leur dossier et leur drapeau.
    "--sans-contrainte-ni-classement-ni-champion": (
        "resultats_sans_contrainte_ni_classement_ni_champion",
        "runs_sans_contrainte_ni_classement_ni_champion", _ABLATION,
        "ablation — sans contrainte, ni classement, ni champion"),
    "--sans-contrainte-ni-classement": (
        "resultats_sans_contrainte_ni_classement",
        "runs_sans_contrainte_ni_classement", _ABLATION,
        "ablation — sans contrainte ni classement"),
    "--sans-contrainte-ni-champion": (
        "resultats_sans_contrainte_ni_champion",
        "runs_sans_contrainte_ni_champion", _ABLATION,
        "ablation — sans contrainte ni champion"),
    "--sans-contrainte": ("resultats_sans_contrainte", "runs_sans_contrainte",
                          _ABLATION, "ablation — sans contrainte de connectivite"),
    "--sans-classement-ni-champion": (
        "resultats_sans_classement_ni_champion",
        "runs_sans_classement_ni_champion", _ABLATION,
        "ablation — sans classement ni champion"),
    "--sans-classement": ("resultats_sans_classement", "runs_sans_classement",
                          _ABLATION, "ablation — sans classement (champion garde)"),
    "--sans-champion": ("resultats_sans_champion", "runs_sans_champion",
                        _ABLATION, "ablation — sans champion (classement garde)"),
    "--temoin": ("resultats_temoin", "runs_temoin", _ABLATION,
                 "temoin — aucune brique retiree"),
    # Anciens dossiers, d'avant la separation classement/champion. Conserves
    # pour pouvoir retracer les campagnes deja produites.
    "--sans-contrainte-ni-info": ("resultats_sans_contrainte_ni_info",
                                  "runs_sans_contrainte_ni_info", _ABLATION,
                                  "ablation — sans contrainte ni classement [ancien nom]"),
    "--sans-info": ("resultats_sans_info", "runs_sans_info", _ABLATION,
                    "ablation — sans classement [ancien nom]"),
}

CAMPAGNE_DEFAUT = "prompt complet — banc de missions"

# Drapeaux qui ne designent pas une campagne.
AUTRES_DRAPEAUX = {"--brut", "--archive"}


def _sans_valeur_archive(argv):
    """argv prive de --archive et de son nom, qui n'est ni tache ni drapeau."""
    out, saute = [], False
    for a in argv:
        if saute:
            saute = False
            continue
        c = a.replace("_", "-")
        if c == "--archive":
            saute = True
            continue
        if c.startswith("--archive="):
            continue
        out.append(a)
    return out


def _lire_archive(argv):
    """Valeur de --archive <nom>, ou None.

    Une campagne archivee garde l'arborescence qu'elle avait :
    archives/<nom>/resultats_complet/, archives/<nom>/runs_complet/, etc.
    On prefixe donc simplement les deux dossiers, et le drapeau de campagne
    continue de choisir lequel lire a l'interieur.
    """
    for i, a in enumerate(argv):
        if a.replace("_", "-") == "--archive":
            if i + 1 >= len(argv) or argv[i + 1].startswith("-"):
                raise SystemExit("[ERREUR] --archive attend un nom : "
                                 "--archive campagne_01_avec_indices")
            return argv[i + 1]
        if a.replace("_", "-").startswith("--archive="):
            return a.split("=", 1)[1]
    return None


def _choisir_campagne(argv):
    """Resout le drapeau de campagne, en refusant ce qui n'en est pas un.

    Avant, un drapeau mal orthographie passait inapercu : le script retombait
    silencieusement sur la campagne par defaut et annoncait qu'aucune mission
    n'avait demarre, alors que les simulations etaient bien la, dans un autre
    dossier. On prefere s'arreter en nommant les drapeaux valides.
    """
    # Les tirets bas sont acceptes : --sans_contrainte_ni_info marche aussi.
    trouves = []
    argv = _sans_valeur_archive(argv)
    for a in [x for x in argv if x.startswith("-")]:
        canon = a.replace("_", "-")
        if canon.startswith("--archive="):
            continue
        if canon in CAMPAGNES:
            trouves.append(canon)
        elif canon not in AUTRES_DRAPEAUX:
            raise SystemExit(
                f"[ERREUR] drapeau inconnu : {a}\n"
                f"         Campagnes disponibles :\n"
                + "".join(f"           {f:28s} -> {CAMPAGNES[f][0]}/\n"
                          for f in CAMPAGNES)
                + f"           {'(aucun drapeau)':28s} -> resultats/\n"
                f"         Autres drapeaux : --brut, --archive <nom>")
    if len(trouves) > 1:
        raise SystemExit(f"[ERREUR] plusieurs campagnes demandees a la fois : "
                         f"{', '.join(trouves)}\n         Choisis-en une seule.")
    return trouves[0] if trouves else None


_FLAG = _choisir_campagne(sys.argv[1:])
_ARCHIVE = _lire_archive(sys.argv[1:])

if _FLAG:
    _RES, _RUNS, SCRIPT, CAMPAGNE = CAMPAGNES[_FLAG]
else:
    _RES, _RUNS, SCRIPT, CAMPAGNE = "resultats", "runs", _MISSIONS, CAMPAGNE_DEFAUT

if _ARCHIVE:
    _base = os.path.join(LLM_DIR, "archives", _ARCHIVE)
    if not os.path.isdir(_base):
        dispo = sorted(os.listdir(os.path.join(LLM_DIR, "archives"))) \
            if os.path.isdir(os.path.join(LLM_DIR, "archives")) else []
        raise SystemExit(
            f"[ERREUR] archive introuvable : archives/{_ARCHIVE}\n"
            + (f"         Archives disponibles :\n"
               + "".join(f"           {d}\n" for d in dispo) if dispo
               else "         Aucune archive dans archives/.\n"
                    "         Les campagnes s'archivent avec archiver_campagne.sh."))
    RESULTS_DIR = os.path.join(_base, _RES)
    RUNS_SUBDIR = os.path.join("archives", _ARCHIVE, _RUNS)
    CAMPAGNE = f"{CAMPAGNE} [archive {_ARCHIVE}]"
else:
    RESULTS_DIR = os.path.join(LLM_DIR, _RES)
    RUNS_SUBDIR = _RUNS

# --brut change la NATURE de la courbe tracee, pas seulement son apparence :
# il faut donc qu'il change aussi le nom du fichier et le badge, sinon les deux
# lectures s'ecrasent l'une l'autre sous le meme nom et plus rien ne dit
# laquelle on regarde.
CUMULATIF = "--brut" not in [a.replace("_", "-") for a in sys.argv[1:]]
MODE = "minimum courant (le meilleur depuis le round 1)" if CUMULATIF \
    else "valeurs brutes (le meilleur de chaque round)"

# Suffixe appose au nom de chaque fichier produit, derive du dossier de sortie
# pour qu'il ne puisse pas diverger : resultats_sans_contrainte_ni_info donne
# sans_contrainte_ni_info, et resultats (la campagne par defaut) donne missions.
# Sans lui, les six campagnes produisent toutes un aggregation_moyenne.png, et
# cinq se font ecraser des qu'on les rassemble dans le dossier figures d'un
# article — sans aucun message.
SUFFIXE = os.path.basename(RESULTS_DIR).replace("resultats", "").lstrip("_") or "missions"
if _ARCHIVE:
    SUFFIXE = f"{SUFFIXE}_archive"
if not CUMULATIF:
    SUFFIXE = f"{SUFFIXE}_brut"

FIG_DIR = os.path.join(RESULTS_DIR, "figures")

COL_A = "#1f4e79"      # irace
COL_B = "#c2551f"      # LLM

# Quantiles de Student a 97.5 % (bilateral 95 %), df = 1..30.
T_975 = {1: 12.706, 2: 4.303, 3: 3.182, 4: 2.776, 5: 2.571, 6: 2.447, 7: 2.365,
         8: 2.306, 9: 2.262, 10: 2.228, 11: 2.201, 12: 2.179, 13: 2.160,
         14: 2.145, 15: 2.131, 16: 2.120, 17: 2.110, 18: 2.101, 19: 2.093,
         20: 2.086, 21: 2.080, 22: 2.074, 23: 2.069, 24: 2.064, 25: 2.060,
         26: 2.056, 27: 2.052, 28: 2.048, 29: 2.045, 30: 2.042}


def t_critical(df: int) -> float:
    if df <= 0:
        return float("nan")
    if df in T_975:
        return T_975[df]
    return 1.960 + 2.40 / df          # approximation normale au-dela de df=30


def mean_ci(values: list):
    """Moyenne et demi-largeur de l'IC bilateral a 95 % (test t)."""
    n = len(values)
    if n == 0:
        return float("nan"), float("nan")
    m = sum(values) / n
    if n == 1:
        return m, float("nan")
    var = sum((v - m) ** 2 for v in values) / (n - 1)
    return m, t_critical(n - 1) * math.sqrt(var / n)


# ---------------------------------------------------------------------------
# Lecture des rapports
# ---------------------------------------------------------------------------
RE_B = re.compile(r"round\s+(\d+)\s*\|\s*best=\s*([-\d.]+)\s*\|\s*origine=(\S+)")
RE_A = re.compile(r"^\s*(\d+)\s*\|\s*([-\d.]+)\s*\|\s*\S+\s*\|\s*\d+\s*\|\s*\d+\s*$", re.M)
HEADER_A = "HISTORIQUE — BRANCHE A"


def mission_run_dir(task: str, idx: int) -> str:
    return os.path.join(LLM_DIR, RUNS_SUBDIR, task, f"mission-{idx:02d}")


def log_branche_a(task: str, idx: int) -> list:
    """Chemins possibles du log irace de la branche A.

    Les deux bras ne nomment pas leurs fichiers pareil : le bras courant ecrit
    irace_A.log, le bras temoin irace_paired_A_<tache>.log, herite du script
    d'origine. On essaie les deux plutot que d'imposer un renommage.
    """
    d = mission_run_dir(task, idx)
    return [os.path.join(d, "irace_A.log"),
            os.path.join(d, f"irace_paired_A_{task}.log")]


def rounds_b_faits(task: str, idx: int) -> int:
    """Rounds de la branche B termines, quel que soit le bras."""
    d = mission_run_dir(task, idx)
    if not os.path.isdir(d):
        return 0
    return sum(1 for f in os.listdir(d)
               if re.fullmatch(r"results_B\d+\.tsv", f)
               or re.fullmatch(rf"paired_B\d+_results_{task}\.tsv", f))


def read_mission(task: str, idx: int):
    """Retourne (serie_A, serie_B, origines) ou None si la mission est absente."""
    report = os.path.join(RESULTS_DIR,
                          f"PAIRED_COMPARISON_{task}_mission-{idx:02d}.txt")
    if not os.path.isfile(report):
        return None
    txt = open(report, errors="ignore").read()

    b_rows = [(int(r), float(v), o) for r, v, o in RE_B.findall(txt)]
    if not b_rows:
        return None
    b_rows.sort()
    serie_b = [v for _, v, _ in b_rows]
    origines = [o for _, _, o in b_rows]

    serie_a = []
    if HEADER_A in txt:
        bloc = txt[txt.index(HEADER_A):]
        bloc = bloc[:bloc.find("HISTORIQUE — BRANCHE B")] if \
            "HISTORIQUE — BRANCHE B" in bloc else bloc
        serie_a = [float(v) for _, v in RE_A.findall(bloc)]
    if not serie_a:
        for log_a in log_branche_a(task, idx):
            serie_a = [r["best"] for r in parse_irace_iterations(log_a)
                       if r["best"] is not None]
            if serie_a:
                break
    if not serie_a:
        return None

    return serie_a, serie_b, origines


def running_min(xs: list) -> list:
    out, best = [], float("inf")
    for x in xs:
        best = min(best, x)
        out.append(best)
    return out


def collect(task: str, cumulatif: bool):
    missions = {}
    for idx in range(1, 51):
        got = read_mission(task, idx)
        if got is None:
            continue
        a, b, orig = got
        n = min(len(a), len(b))
        if n == 0:
            continue
        missions[idx] = dict(a=a[:n], b=(running_min(b) if cumulatif else b)[:n],
                             origines=orig[:n], n=n)
    return missions


# ---------------------------------------------------------------------------
# Figures
# ---------------------------------------------------------------------------
def badge_campagne(fig, reserve_pouces=0.62):
    """Inscrit la campagne et le mode de lecture en haut a gauche.

    Place hors des axes, donc jamais recouvert par les courbes, et conserve
    quand la figure est recadree ou collee dans un article. Le contenu est
    d'abord comprime vers le bas, sinon le badge chevauche le titre des que le
    libelle s'allonge (campagne + archive).

    La reserve est exprimee en POUCES, pas en fraction : une grille de douze
    vignettes fait trois fois la hauteur d'une courbe seule, et une fraction
    fixe y creusait un bandeau blanc enorme.
    """
    fig.subplots_adjust(top=fig.subplotpars.top
                        - reserve_pouces / fig.get_figheight())
    fig.text(0.008, 0.992, f"{CAMPAGNE}\n{MODE}", ha="left", va="top",
             fontsize=8, color="#1a1a1a", linespacing=1.5,
             bbox=dict(boxstyle="round,pad=0.35", facecolor="#eceff3",
                       edgecolor="#9aa4b0", linewidth=0.6))


def fig_par_mission(task: str, missions: dict, path: str):
    ids = sorted(missions)
    ncol = min(4, len(ids))
    nrow = math.ceil(len(ids) / ncol)
    fig, axes = plt.subplots(nrow, ncol, figsize=(4 * ncol, 3 * nrow),
                             squeeze=False)
    for ax, idx in zip([a for row in axes for a in row], ids):
        d = missions[idx]
        x = range(1, d["n"] + 1)
        ax.plot(x, d["a"], "-o", color=COL_A, ms=3.5, lw=1.6, label="irace")
        ax.plot(x, d["b"], "-s", color=COL_B, ms=3.5, lw=1.6, label="LLM")
        irace_pts = [(r, v) for r, (v, o) in
                     enumerate(zip(d["b"], d["origines"]), 1) if o == "irace"]
        if irace_pts:
            ax.plot([r for r, _ in irace_pts], [v for _, v in irace_pts],
                    "x", color="black", ms=7, mew=1.6,
                    label="champion B issu d'irace")
        ax.set_title(f"mission-{idx:02d}", fontsize=10)
        ax.grid(alpha=.3)
        ax.tick_params(labelsize=8)
    for ax in [a for row in axes for a in row][len(ids):]:
        ax.axis("off")
    axes[0][0].legend(fontsize=8)
    fig.suptitle(f"{task.upper()} — meilleur score par round "
                 f"(plus bas = meilleur), {len(ids)} missions", fontsize=13)
    fig.supxlabel("round / iteration", fontsize=10)
    fig.supylabel("score", fontsize=10)
    fig.tight_layout()
    badge_campagne(fig)
    fig.savefig(path, dpi=150)
    plt.close(fig)


def _courbe_moyenne(ax, rounds, stats_a, stats_b, ylabel):
    ma, ea = zip(*stats_a)
    mb, eb = zip(*stats_b)
    for m, e, col, lab in ((ma, ea, COL_A, "irace"), (mb, eb, COL_B, "LLM")):
        ax.plot(rounds, m, "-o", color=col, ms=4, lw=2, label=lab)
        lo = [v - (d if d == d else 0) for v, d in zip(m, e)]
        hi = [v + (d if d == d else 0) for v, d in zip(m, e)]
        ax.fill_between(rounds, lo, hi, color=col, alpha=.18, linewidth=0)
    ax.set_xlabel("round / iteration")
    ax.set_ylabel(ylabel)
    ax.grid(alpha=.3)
    ax.legend()


def fig_moyenne(task, missions, path, normalise=False):
    ids = sorted(missions)
    nmax = max(missions[i]["n"] for i in ids)
    rounds, stats_a, stats_b, ns = [], [], [], []
    for r in range(nmax):
        va = [missions[i]["a"][r] for i in ids if r < missions[i]["n"]]
        vb = [missions[i]["b"][r] for i in ids if r < missions[i]["n"]]
        if normalise:
            va = [missions[i]["a"][r] / missions[i]["a"][0]
                  for i in ids if r < missions[i]["n"] and missions[i]["a"][0]]
            vb = [missions[i]["b"][r] / missions[i]["b"][0]
                  for i in ids if r < missions[i]["n"] and missions[i]["b"][0]]
        if not va or not vb:
            continue
        rounds.append(r + 1)
        stats_a.append(mean_ci(va))
        stats_b.append(mean_ci(vb))
        ns.append(min(len(va), len(vb)))

    fig, ax = plt.subplots(figsize=(8, 5))
    _courbe_moyenne(ax, rounds, stats_a, stats_b,
                    "score relatif au round 1" if normalise else "score moyen")
    if normalise:
        ax.axhline(1.0, color="grey", lw=.8, ls=":")
    titre = "moyenne normalisee" if normalise else "moyenne des missions"
    ax.set_title(f"{task.upper()} — {titre}\n"
                 f"n = {ns[0] if ns else 0} missions, bande = IC bilateral 95 % (test t)",
                 fontsize=12)
    if CUMULATIF:
        note = ("irace : moyenne de son elite reevaluee.  LLM : minimum courant du "
                "meilleur de chaque round.\nLe minimum d'estimations bruitees est "
                "optimiste : conclure demande une evaluation hors ligne.")
    else:
        note = ("irace : moyenne de son elite reevaluee.  LLM : meilleur de CHAQUE "
                "round, sans minimum courant.\nUne courbe qui remonte signale une "
                "recherche qui n'a pas converge.")
    fig.text(.5, .005, note, ha="center", fontsize=7.5, color="#555555")
    fig.tight_layout(rect=(0, .05, 1, 1))
    badge_campagne(fig)
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return rounds, stats_a, stats_b, ns


def fig_difference(task, missions, path):
    ids = sorted(missions)
    nmax = max(missions[i]["n"] for i in ids)
    rounds, means, errs, ns = [], [], [], []
    for r in range(nmax):
        diffs = [missions[i]["b"][r] - missions[i]["a"][r]
                 for i in ids if r < missions[i]["n"]]
        if len(diffs) < 2:
            continue
        m, e = mean_ci(diffs)
        rounds.append(r + 1)
        means.append(m)
        errs.append(e)
        ns.append(len(diffs))

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.axhline(0, color="black", lw=1)
    ax.errorbar(rounds, means, yerr=errs, fmt="-o", color="#4a2b8c",
                ms=5, lw=2, capsize=4)
    ax.fill_between(rounds, [m - e for m, e in zip(means, errs)],
                    [m + e for m, e in zip(means, errs)],
                    color="#4a2b8c", alpha=.15, linewidth=0)
    ax.set_xlabel("round / iteration")
    ax.set_ylabel("difference appariee  B − A")
    ax.grid(alpha=.3)
    ax.set_title(f"{task.upper()} — difference appariee LLM − irace\n"
                 f"n = {ns[0] if ns else 0} missions, barres = IC bilateral 95 % (test t)",
                 fontsize=12)
    ax.text(.02, .04, "sous la ligne zero : le LLM fait mieux",
            transform=ax.transAxes, fontsize=9, color="#444444")
    fig.tight_layout()
    badge_campagne(fig)
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return rounds, means, errs, ns


def ecrire_tsv(path, rounds, sa, sb, diff, ns):
    with open(path, "w") as f:
        f.write(f"# campagne : {CAMPAGNE}  ({os.path.basename(RESULTS_DIR)}/)\n")
        f.write("round\tn_missions\tirace_moy\tirace_ic95\tllm_moy\tllm_ic95\t"
                "diff_moy\tdiff_ic95\tllm_meilleur_significatif\n")
        dmap = {r: (m, e) for r, m, e in zip(diff[0], diff[1], diff[2])}
        for i, r in enumerate(rounds):
            dm, de = dmap.get(r, (float("nan"), float("nan")))
            sig = ""
            if de == de and dm == dm:
                sig = "oui" if dm + de < 0 else ("non (A meilleur)" if dm - de > 0 else "non")
            f.write(f"{r}\t{ns[i]}\t{sa[i][0]:.3f}\t{sa[i][1]:.3f}\t"
                    f"{sb[i][0]:.3f}\t{sb[i][1]:.3f}\t{dm:.3f}\t{de:.3f}\t{sig}\n")


def signaler_anciennes_figures(task: str):
    """Previent si des figures sans suffixe de campagne trainent encore.

    Elles datent d'avant l'ajout du suffixe et ne portent ni badge ni nom de
    campagne : les reprendre dans un article reviendrait a publier une figure
    dont plus personne ne sait de quelle campagne elle sort.
    """
    if not os.path.isdir(FIG_DIR):
        return
    anciennes = [f for f in sorted(os.listdir(FIG_DIR))
                 if re.fullmatch(rf"{re.escape(task)}_"
                                 r"(par_mission|moyenne|normalise|difference)\.png"
                                 rf"|{re.escape(task)}_donnees\.tsv", f)]
    if not anciennes:
        return
    print(f"\n  [ATTENTION] {len(anciennes)} fichier(s) d'avant l'ajout du suffixe "
          f"subsistent ici :")
    for f in anciennes:
        print(f"                {f}")
    print(f"              Ils ne portent ni badge ni nom de campagne. Supprime-les "
          f"pour ne pas\n"
          f"              les confondre avec les nouveaux :\n"
          f"                cd {FIG_DIR} && rm " + " ".join(anciennes))


# ---------------------------------------------------------------------------
def main():
    args = _sans_valeur_archive(sys.argv[1:])
    cumulatif = CUMULATIF
    wanted = [a for a in args if not a.startswith("-")] or TASKS
    os.makedirs(FIG_DIR, exist_ok=True)

    for task in wanted:
        missions = collect(task, cumulatif)
        if not missions:
            # Un rapport n'est ecrit qu'a la FIN d'une mission complete
            # (etape 0 + branche A + les 10 rounds de la branche B). Tant
            # qu'aucune n'est terminee, il n'y a rien a tracer : on dit ou
            # en sont les missions plutot que de laisser l'utilisateur
            # chercher ce qui ne va pas.
            print(f"[{task}] aucune mission terminee : rien a tracer pour l'instant.")
            runs = os.path.join(LLM_DIR, RUNS_SUBDIR, task)
            if not os.path.isdir(runs):
                print(f"         Aucun dossier {RUNS_SUBDIR}/{task} : la campagne n'a pas demarre.")
                print(f"         Lance : python3 {SCRIPT} {task}")
                continue
            en_cours = []
            for i in range(1, 51):
                if not os.path.isdir(mission_run_dir(task, i)):
                    continue
                it_a = 0
                for la in log_branche_a(task, i):
                    it_a = len(parse_irace_iterations(la))
                    if it_a:
                        break
                en_cours.append(dict(
                    name=f"mission-{i:02d}",
                    detail=f"branche A : {it_a}/10 iterations | "
                           f"branche B : {rounds_b_faits(task, i)}/10 rounds"))
            if not en_cours:
                print(f"         {RUNS_SUBDIR}/{task} existe mais ne contient aucune "
                      f"mission.\n"
                      f"         Regarde la sortie du script : l'etape 0 a peut-etre "
                      f"echoue sur chaque mission.")
                continue
            print(f"         {len(en_cours)} mission(s) en cours :")
            for e in en_cours[:6]:
                print(f"           {e['name']}  {e['detail']}")
            if len(en_cours) > 6:
                print(f"           ... et {len(en_cours) - 6} autre(s)")
            print(f"\n         Une mission complete demande l'etape 0, les 6000 evaluations")
            print(f"         de la branche A puis les 10 rounds de la branche B. Reviens")
            print(f"         quand la premiere est terminee, ou suis l'avancement avec :")
            print(f"           python3 {SCRIPT} {task} --status")
            continue

        print(f"\n=== {task} [{CAMPAGNE}] — {len(missions)} mission(s) : "
              f"{', '.join(f'{i:02d}' for i in sorted(missions))} ===")
        if len(missions) < 2:
            print("  (une seule mission : pas d'intervalle de confiance possible, "
                  "seule la figure par mission sera produite)")

        p1 = os.path.join(FIG_DIR, f"{task}_par_mission_{SUFFIXE}.png")
        fig_par_mission(task, missions, p1)
        print(f"  {p1}")

        if len(missions) < 2:
            continue

        p2 = os.path.join(FIG_DIR, f"{task}_moyenne_{SUFFIXE}.png")
        rounds, sa, sb, ns = fig_moyenne(task, missions, p2)
        print(f"  {p2}")

        p3 = os.path.join(FIG_DIR, f"{task}_normalise_{SUFFIXE}.png")
        fig_moyenne(task, missions, p3, normalise=True)
        print(f"  {p3}")

        p4 = os.path.join(FIG_DIR, f"{task}_difference_{SUFFIXE}.png")
        diff = fig_difference(task, missions, p4)
        print(f"  {p4}")

        p5 = os.path.join(FIG_DIR, f"{task}_donnees_{SUFFIXE}.tsv")
        ecrire_tsv(p5, rounds, sa, sb, diff, ns)
        print(f"  {p5}")

        signaler_anciennes_figures(task)

        dm, de = diff[1][-1], diff[2][-1]
        verdict = ("le LLM est significativement meilleur" if dm + de < 0 else
                   "irace est significativement meilleur" if dm - de > 0 else
                   "aucun ecart significatif")
        print(f"  -> au dernier round : B - A = {dm:+.1f} "
              f"[{dm - de:+.1f} ; {dm + de:+.1f}]  ({verdict})")


if __name__ == "__main__":
    main()
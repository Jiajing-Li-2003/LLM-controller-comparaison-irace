#!/usr/bin/env bash
#
# setup_base.sh — prepare le depot de base pour lancer les comparaisons.
#
#     bash setup_base.sh
#
# Ce script :
#   1. deballe irace-foraging.tar.gz (la mission foraging n'existe que sous
#      forme d'archive dans le depot de base) ;
#   2. recale les chemins absolus des .argos et des target-runner sur
#      l'emplacement reel du depot ;
#   3. cree les execution-folder manquants ;
#   4. fait un TEST DE FUMEE : il execute reellement une FSM triviale via le
#      target-runner de chaque mission, exactement comme le fera irace.
#
# Il est IDEMPOTENT et REPARATEUR : le relancer autant de fois qu'on veut ne
# casse rien, et il repare un depot dont les chemins ont ete abimes par une
# version precedente de ce script.
#
# Il NE compile rien : suis installation-instructions.txt pour ARGoS3, les
# plugins e-puck/arena/DAO, les loop-functions et AutoMoDe.

set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"

# --- racine du depot --------------------------------------------------------
find_root() {
    local d="$1"
    while [ "$d" != "/" ]; do
        if [ -d "$d/software/argos3-installation/automode/irace-files" ]; then
            echo "$d"
            return 0
        fi
        d="$(dirname "$d")"
    done
    return 1
}

if [ -n "${MANDARINA_ROOT:-}" ]; then
    ROOT="$MANDARINA_ROOT"
else
    ROOT="$(find_root "$HERE")" || {
        echo "[ERREUR] Racine du depot introuvable en remontant depuis :"
        echo "           $HERE"
        echo "         Aucun dossier parent ne contient"
        echo "           software/argos3-installation/automode/irace-files"
        echo
        echo "         Corrige avec :"
        echo "           export MANDARINA_ROOT=/chemin/vers/le/depot"
        exit 1
    }
fi

ROOT="${ROOT%/}"
IRACE_FILES="$ROOT/software/argos3-installation/automode/irace-files"
INSTALL="$ROOT/software/argos3-installation"

echo "Depot de base : $ROOT"
[ -d "$IRACE_FILES" ] || {
    echo "[ERREUR] $IRACE_FILES introuvable."
    echo "         MANDARINA_ROOT pointe-t-il bien sur la racine du depot ?"
    exit 1
}

# --- 1. deballer foraging ---------------------------------------------------
if [ ! -d "$IRACE_FILES/irace-foraging" ] && [ -f "$IRACE_FILES/irace-foraging.tar.gz" ]; then
    echo "-> deballage de irace-foraging.tar.gz"
    mkdir -p "$IRACE_FILES/irace-foraging"
    tar -xzf "$IRACE_FILES/irace-foraging.tar.gz" -C "$IRACE_FILES/irace-foraging"
fi

# --- 2. recaler les chemins absolus ----------------------------------------
#
# Les trois regles reecrivent le PREFIXE quel qu'il soit, au lieu de remplacer
# une chaine fixe. Consequence : relancer le script ne peut pas empiler les
# substitutions, et un chemin deja abime est repare.
#
# Chaque motif commence par '/' pour ne pas manger le 'EXE=' d'une affectation
# shell ni le 'library="' d'un attribut XML.
ESC_ROOT=$(printf '%s' "$ROOT" | sed -e 's/[&\\#]/\\&/g')

echo "-> recalage des chemins absolus"
CHANGED=0
for f in "$IRACE_FILES"/irace-*/experiments-folder/*.argos "$IRACE_FILES"/irace-*/target-runner; do
    [ -f "$f" ] || continue
    BEFORE=$(cat "$f")
    sed -E -i \
        -e "s#/[^\"' ]*/AutoMoDe-tuttifrutti/#$ESC_ROOT/software/argos3-installation/automode/#g" \
        -e "s#/[^\"' ]*/experiments-loop-functions/#$ESC_ROOT/software/argos3-installation/experiments-loop-functions/#g" \
        -e "s#/[^\"' ]*/software/argos3-installation/#$ESC_ROOT/software/argos3-installation/#g" \
        "$f"
    [ "$BEFORE" != "$(cat "$f")" ] && CHANGED=$((CHANGED + 1))
done
echo "   $CHANGED fichier(s) modifie(s)"

# --- 2 bis. loop function coherente avec le label --------------------------
#
# Defaut de l'archive de base : irace-stop/experiments-folder/sequence-1.argos
# charge libtutti_AG_loopfunc.so tout en demandant le label
# "tutti_st_loop_function". La bibliotheque chargee n'enregistre pas ce label.
# Selon ce qui traine dans ARGOS_PLUGIN_PATH, ARGoS refuse de demarrer ou
# resout le label ailleurs — dans les deux cas la mission stop ne mesure pas ce
# qu'elle devrait. On force donc la bibliotheque a correspondre au label.
echo "-> coherence loop function / label"
declare -A LOOPLIB=( [aggregation]=ag [stop]=st [foraging]=fr )
for t in aggregation stop foraging; do
    SHORT="${LOOPLIB[$t]}"
    for A in "$IRACE_FILES/irace-$t/experiments-folder"/*.argos; do
        [ -f "$A" ] || continue
        WAS=$(grep -oE 'libtutti_[a-z]+_loopfunc\.so' "$A" | head -1)
        sed -E -i "s#libtutti_[a-z]+_loopfunc\.so#libtutti_${SHORT}_loopfunc.so#g" "$A"
        NOW=$(grep -oE 'libtutti_[a-z]+_loopfunc\.so' "$A" | head -1)
        if [ "$WAS" != "$NOW" ]; then
            echo "   $t : $WAS -> $NOW (corrige)"
        fi
    done
done

# --- 2 ter. passage des arguments au binaire -------------------------------
#
# Defaut de l'archive de base : le target-runner d'aggregation passe la
# configuration FSM ENTRE GUILLEMETS,
#     --fsm-config "${FSM_CONFIG}"
# ce qui la livre a automode_main comme UN SEUL argument contenant des espaces.
# Le constructeur de FSM ne trouve alors aucun jeton et leve
#     vector::_M_range_check: __n (which is 0) >= this->size() (which is 0)
# Le target-runner intercepte, renvoie 1e10, et irace croit que toutes les
# configurations sont mauvaises. Les target-runner de stop et foraging passent
# la variable sans guillemets et fonctionnent : on aligne aggregation dessus.
echo "-> passage des arguments FSM au binaire"
for t in aggregation stop foraging; do
    TRF="$IRACE_FILES/irace-$t/target-runner"
    [ -f "$TRF" ] || continue
    if grep -qE -- '--fsm-config[[:space:]]+"\$' "$TRF"; then
        sed -E -i 's#(--fsm-config[[:space:]]+)"(\$\{?[A-Za-z_]+\}?)"#\1\2#' "$TRF"
        echo "   $t : --fsm-config \"\$VAR\" -> --fsm-config \$VAR (corrige)"
    fi
done

# --- 2 quater. bornage des destinations de transition ----------------------
#
# Defaut de l'archive de base : la grammaire declare
#     N0x0 "--n0x0 " i (0,3)
# quel que soit le nombre d'etats, alors que la destination est un indice
# PARMI LES AUTRES etats : avec --nstates 2 la seule valeur valide est 0.
# irace tire donc regulierement 1, 2 ou 3, et AutoMoDe repond
#     [FATAL] Could not create the Finite State Machine: Error while parsing.
# Le fichier grammars/sq-depend-config.txt declare bien la borne
# "N0x0 (|NumStates-2)", mais aucun scenario ne le charge : irace l'ignore.
# On borne donc dans le target-runner, juste avant l'appel au binaire.
echo "-> bornage des destinations de transition"
for t in aggregation stop foraging; do
    TRF="$IRACE_FILES/irace-$t/target-runner"
    [ -f "$TRF" ] || continue
    grep -q "LLM_MISSION_CLAMP" "$TRF" && continue
    python3 - "$TRF" <<'PYEOF'
import re, sys
path = sys.argv[1]
src = open(path).read()
clamp = """
# --- LLM_MISSION_CLAMP : borne les destinations a [0, nstates-2] -------------
# La destination d'une transition est un indice parmi les AUTRES etats, donc
# elle vaut au plus nstates-2. La grammaire irace ne connait pas cette borne.
CONFIG_PARAMS=$(printf '%s' "$CONFIG_PARAMS" | awk '{
    n = 1
    for (i = 1; i <= NF; i++) if ($i == "--nstates") { n = $(i+1) + 0; break }
    max = n - 2; if (max < 0) max = 0
    for (i = 1; i < NF; i++)
        if ($i ~ /^--n[0-9]+x[0-9]+$/) {
            v = $(i+1) + 0
            if (v > max) v = max
            if (v < 0) v = 0
            $(i+1) = v
        }
    print
}')
# --- fin LLM_MISSION_CLAMP --------------------------------------------------
"""
m = re.search(r'^CONFIG_PARAMS=.*$', src, re.M)
if not m:
    sys.exit("CONFIG_PARAMS introuvable")
src = src[:m.end()] + "\n" + clamp + src[m.end():]
open(path, "w").write(src)
PYEOF
    echo "   $t : bornage insere"
done

# --- 3. dossiers d'execution ------------------------------------------------
for d in "$IRACE_FILES"/irace-*/; do
    mkdir -p "$d/execution-folder"
done

# =============================================================================
#                              VERIFICATIONS
# =============================================================================
FAIL=0
echo
echo "--- binaires et bibliotheques ---"

EXE="$INSTALL/automode/bin/automode_main"
if [ -x "$EXE" ]; then
    echo "  [ok] automode_main"
else
    echo "  [MANQUE] $EXE"
    echo "           compile AutoMoDe (voir installation-instructions.txt)"
    FAIL=1
fi

for lib in tutti_ag tutti_st tutti_fr; do
    L="$INSTALL/experiments-loop-functions/build/loop-functions/moca/lib${lib}_loopfunc.so"
    if [ -f "$L" ]; then
        echo "  [ok] lib${lib}_loopfunc.so"
    else
        echo "  [MANQUE] lib${lib}_loopfunc.so"
        FAIL=1
    fi
done

if ! command -v Rscript >/dev/null 2>&1; then
    echo "  [MANQUE] Rscript — installe R, puis dans R : install.packages(\"irace\")"
    FAIL=1
elif Rscript -e 'q(status = !nzchar(system.file("bin","irace",package="irace")))' >/dev/null 2>&1; then
    echo "  [ok] package R irace"
else
    echo "  [MANQUE] package irace — dans R : install.packages(\"irace\")"
    FAIL=1
fi

# --- chemins effectivement inscrits dans les fichiers -----------------------
echo
echo "--- chemins inscrits dans les fichiers ---"
for t in aggregation stop foraging; do
    TR="$IRACE_FILES/irace-$t/target-runner"
    if [ ! -f "$TR" ]; then
        echo "  [MANQUE] target-runner de $t"
        FAIL=1
        continue
    fi

    if grep -qE -- '--fsm-config[[:space:]]+"\$' "$TR"; then
        echo "  [CASSE] $t : le target-runner passe --fsm-config entre guillemets,"
        echo "          la FSM arrivera comme un seul argument et le binaire plantera."
        FAIL=1
    fi

    if ! grep -q "LLM_MISSION_CLAMP" "$TR"; then
        echo "  [CASSE] $t : le target-runner ne borne pas les destinations de"
        echo "          transition ; irace produira des FSM que AutoMoDe refuse."
        FAIL=1
    fi

    TR_EXE=$(grep -m1 -oE '^EXE=[^ ]+' "$TR" | cut -d= -f2-)
    if [ -x "$TR_EXE" ]; then
        echo "  [ok] $t : le target-runner pointe sur un binaire executable"
    else
        echo "  [CASSE] $t : le target-runner pointe sur"
        echo "            $TR_EXE"
        echo "          qui n'existe pas ou n'est pas executable."
        FAIL=1
    fi

    EXPECTED="libtutti_$(case $t in aggregation) echo ag;; stop) echo st;; foraging) echo fr;; esac)_loopfunc.so"
    for A in "$IRACE_FILES/irace-$t/experiments-folder"/*.argos; do
        [ -f "$A" ] || continue
        GOT=$(grep -oE 'libtutti_[a-z]+_loopfunc\.so' "$A" | head -1)
        if [ "$GOT" != "$EXPECTED" ]; then
            echo "  [CASSE] $t : $(basename "$A") charge $GOT au lieu de $EXPECTED"
            FAIL=1
        fi
        while IFS= read -r LIBPATH; do
            [ -n "$LIBPATH" ] || continue
            if [ ! -f "$LIBPATH" ]; then
                echo "  [CASSE] $t : bibliotheque introuvable dans $(basename "$A")"
                echo "            $LIBPATH"
                FAIL=1
            fi
        done < <(grep -oE 'library="[^"]+"' "$A" | sed -e 's/library="//' -e 's/"$//')
    done
done

# --- test de fumee : on execute vraiment -----------------------------------
if [ "$FAIL" -eq 0 ]; then
    echo
    echo "--- test de fumee (execution reelle d'une FSM triviale) ---"
    for t in aggregation stop foraging; do
        TR="$IRACE_FILES/irace-$t/target-runner"
        INST="$IRACE_FILES/irace-$t/experiments-folder/sequence-1.argos"
        EXECDIR="$IRACE_FILES/irace-$t/execution-folder"
        if [ ! -f "$INST" ]; then
            echo "  [MANQUE] instance de reference de $t"
            FAIL=1
            continue
        fi

        # stdout et stderr separes : en cas d'echec le target-runner recopie
        # toute la sortie d'ARGoS sur stderr, et c'est la qu'est la vraie cause.
        ERRFILE=$(mktemp)
        OUT=$(cd "$EXECDIR" && QT_QPA_PLATFORM=offscreen DISPLAY= \
              bash "$TR" 1 1 1234 "$INST" --nstates 1 --s0 1 --cle0 0 2>"$ERRFILE" | tail -n 1)

        case "$OUT" in
            1e10|''|*[!0-9.eE+-]*)
                echo "  [ECHEC] $t : pas de score exploitable (sortie : '${OUT:-vide}')."
                echo "          --- ce qu'ARGoS a reellement dit ---"
                grep -vE '^\[INFO\]|^$' "$ERRFILE" | tail -n 15 | sed 's/^/          /'
                echo "          ------------------------------------"
                echo "          Pour creuser, rejoue automode_main sans le target-runner :"
                echo "            cd $EXECDIR"
                echo "            $INSTALL/automode/bin/automode_main -c $INST \\"
                echo "                --seed 1234 --fsm-config --nstates 1 --s0 1 --cle0 0"
                FAIL=1
                ;;
            *)
                echo "  [ok] $t : score obtenu = $OUT"
                ;;
        esac
        rm -f "$ERRFILE"
    done
fi

echo
if [ "$FAIL" -eq 0 ]; then
    echo "Tout est pret."
    echo
    echo "  export ANTHROPIC_API_KEY=..."
    echo "  python3 run_paired_comparison_d_missions.py aggregation"
else
    echo "[ATTENTION] Des points bloquants subsistent (voir ci-dessus)."
    echo "            Ne lance pas la campagne tant qu'ils ne sont pas regles :"
    echo "            irace tournerait en renvoyant 1e10 pour toutes les configurations."
    exit 1
fi

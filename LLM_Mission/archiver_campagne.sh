#!/usr/bin/env bash
#
# archiver_campagne.sh — met de cote une ou plusieurs campagnes terminees avant
# d'en relancer une autre, au lieu de les effacer.
#
#     bash archiver_campagne.sh <tache> <nom-archive> [campagnes...] [--avec-banc]
#
#     bash archiver_campagne.sh aggregation camp_01 --complet
#     bash archiver_campagne.sh foraging    lot_02  --complet --sans-contrainte-ni-info
#     bash archiver_campagne.sh stop        lot_03  --toutes
#
# Campagnes reconnues, chacune avec sa paire de dossiers :
#
#     (aucun drapeau)             resultats/                        runs/
#     --origin                    resultats_origin/                 runs_origin/
#     --complet                   resultats_complet/                runs_complet/
#     --sans-contrainte-ni-info   resultats_sans_contrainte_ni_info/  ...
#     --sans-contrainte           resultats_sans_contrainte/          ...
#     --sans-info                 resultats_sans_info/                ...
#     --toutes                    toutes celles qui ont des donnees pour la tache
#
# Les tirets bas sont acceptes. PLUSIEURS campagnes peuvent aller dans la meme
# archive : chacune garde son sous-dossier, donc
#   python3 plot_comparaison.py <tache> --archive <nom> --complet
# sait encore laquelle lire.
#
# Deplace les rapports, journaux, recapitulatifs, prompts systeme, figures et
# tout le detail des executions, puis note dans INFO.txt la date, les campagnes
# et l'etat du depot.
#
# Rien n'est supprime : la tache repart de zero parce que ses fichiers ont
# change de place, et l'ancienne campagne reste comparable a la nouvelle.

set -uo pipefail

# --- catalogue des campagnes ------------------------------------------------
# Une etiquette par interrupteur d'ablation. Les deux dernieres sont les
# anciens noms, d'avant la separation classement/champion : conservees pour
# que les campagnes deja produites restent archivables.
CLES=(courante origin complet temoin
      sans_contrainte
      sans_classement
      sans_champion
      sans_contrainte_ni_classement
      sans_contrainte_ni_champion
      sans_classement_ni_champion
      sans_contrainte_ni_classement_ni_champion
      sans_contrainte_ni_info
      sans_info)

dossier_res() {
    case "$1" in
        courante) echo "resultats" ;;
        *)        echo "resultats_$1" ;;
    esac
}
dossier_runs() {
    case "$1" in
        courante) echo "runs" ;;
        *)        echo "runs_$1" ;;
    esac
}
a_un_banc() {   # seules les campagnes sur banc de missions en ont un
    case "$1" in
        courante|origin) return 0 ;;
        *)               return 1 ;;   # complet, temoin et toutes les ablations
    esac                               # tournent sur l'instance publiee
}

# --- arguments : les drapeaux peuvent etre n'importe ou ---------------------
TASK=""
NAME=""
WITH_BENCH="no"
TOUTES="no"
DEMANDEES=()

for arg in "$@"; do
    canon="${arg//_/-}"
    case "$canon" in
        --avec-banc) WITH_BENCH="yes" ;;
        --toutes)    TOUTES="yes" ;;
        --*)
            # --<cle avec des tirets> : un drapeau par entree de CLES, sans
            # avoir a les lister deux fois.
            cle="${canon#--}"; cle="${cle//-/_}"
            connu="non"
            for k in "${CLES[@]}"; do
                [ "$k" = "$cle" ] && connu="oui" && break
            done
            if [ "$connu" = "oui" ]; then
                DEMANDEES+=("$cle")
            else
                echo "[ERREUR] drapeau inconnu : $arg"
                echo "         Campagnes :"
                for k in "${CLES[@]}"; do printf "           --%s\n" "${k//_/-}"; done
                echo "           --toutes"
                echo "         Autre : --avec-banc"
                exit 1
            fi ;;
        *)  if [ -z "$TASK" ]; then TASK="$arg"
            elif [ -z "$NAME" ]; then NAME="$arg"
            else echo "[ERREUR] argument en trop : $arg"; exit 1; fi ;;
    esac
done

if [ -z "$TASK" ] || [ -z "$NAME" ]; then
    echo "Usage : bash archiver_campagne.sh <tache> <nom-archive> [campagnes...] [--avec-banc]"
    echo "Exemple : bash archiver_campagne.sh foraging lot_02 --complet --sans-contrainte-ni-info"
    exit 1
fi

HERE="$(cd "$(dirname "$0")" && pwd)"
DEST="$HERE/archives/$NAME"

# --- quelles campagnes ont REELLEMENT des donnees pour cette tache ? --------
# C'est la question qui evitait l'archive vide : avant, lancer le script sans
# drapeau archivait la campagne par defaut, qui pouvait n'avoir aucun fichier
# pour cette tache, et on repartait avec "0 element(s)" sans savoir pourquoi.
a_des_donnees() {
    local cle="$1"
    local res="$HERE/$(dossier_res "$cle")"
    local runs="$HERE/$(dossier_runs "$cle")/$TASK"
    [ -d "$runs" ] && return 0
    ls "$res"/PAIRED_COMPARISON_"${TASK}"_mission-*.txt >/dev/null 2>&1 && return 0
    return 1
}

AVEC_DONNEES=()
for c in "${CLES[@]}"; do
    a_des_donnees "$c" && AVEC_DONNEES+=("$c")
done

if [ "$TOUTES" = "yes" ]; then
    DEMANDEES=("${AVEC_DONNEES[@]}")
elif [ ${#DEMANDEES[@]} -eq 0 ]; then
    DEMANDEES=(courante)
fi

# --- rien a archiver : on le dit AVANT de creer un dossier vide -------------
RETENUES=()
for c in "${DEMANDEES[@]}"; do
    if a_des_donnees "$c"; then
        RETENUES+=("$c")
    else
        echo "[INFO] campagne '$c' : rien pour la tache '$TASK', ignoree."
    fi
done

if [ ${#RETENUES[@]} -eq 0 ]; then
    echo
    echo "[ERREUR] aucune des campagnes demandees n'a de donnees pour '$TASK'."
    if [ ${#AVEC_DONNEES[@]} -eq 0 ]; then
        echo "         Aucune campagne n'a de donnees pour cette tache."
    else
        echo "         Campagnes qui en ont :"
        for c in "${AVEC_DONNEES[@]}"; do
            printf "           --%-26s -> %s/\n" "${c//_/-}" "$(dossier_res "$c")"
        done
        echo
        echo "         Exemple :"
        echo "           bash archiver_campagne.sh $TASK $NAME --${AVEC_DONNEES[0]//_/-}"
    fi
    exit 1
fi

if [ -e "$DEST" ]; then
    echo "[ERREUR] $DEST existe deja. Choisis un autre nom,"
    echo "         ou supprime-le s'il est vide :  rmdir \"$DEST\""
    exit 1
fi

# --- deplacement ------------------------------------------------------------
MOVED=0

move_if_any() {
    local pattern="$1" target="$2"
    local n=0
    # nullglob vide les motifs a jokers, mais un chemin SANS joker (RECAP,
    # prompt systeme) reste tel quel meme si le fichier n'existe pas : d'ou
    # le test d'existence, sinon mv se plaint pour rien.
    shopt -s nullglob
    for f in $pattern; do
        [ -e "$f" ] || continue
        mkdir -p "$target"
        mv "$f" "$target/" && n=$((n + 1))
    done
    shopt -u nullglob
    MOVED=$((MOVED + n))
    echo "   $n element(s)"
}

TASK_MAJ="$(echo "$TASK" | tr '[:lower:]' '[:upper:]')"
mkdir -p "$DEST"

for c in "${RETENUES[@]}"; do
    RES="$(dossier_res "$c")"
    RUNS="$(dossier_runs "$c")"
    echo
    echo "--- campagne '$c'  ($RES/ et $RUNS/) ---"
    echo -n " rapports        :"; move_if_any "$HERE/$RES/PAIRED_COMPARISON_${TASK}_mission-*.txt" "$DEST/$RES"
    echo -n " journaux        :"; move_if_any "$HERE/$RES/${TASK_MAJ}_mission-*.txt" "$DEST/$RES"
    echo -n " recapitulatif   :"; move_if_any "$HERE/$RES/RECAP_${TASK}.tsv" "$DEST/$RES"
    echo -n " prompt systeme  :"; move_if_any "$HERE/$RES/system_prompt_${TASK}.txt" "$DEST/$RES"
    echo -n " figures         :"; move_if_any "$HERE/$RES/figures/${TASK}_*" "$DEST/$RES/figures"
    echo -n " executions      :"; move_if_any "$HERE/$RUNS/$TASK" "$DEST/$RUNS"
done

# --- banc d'instances -------------------------------------------------------
BESOIN_BANC="non"
for c in "${RETENUES[@]}"; do
    a_un_banc "$c" && BESOIN_BANC="oui"
done

echo
if [ "$BESOIN_BANC" = "non" ]; then
    echo " banc d'instances: aucun — ces campagnes tournent sur l'instance publiee"
    WITH_BENCH="sans objet"
elif [ "$WITH_BENCH" = "yes" ]; then
    echo -n " banc d'instances:"; move_if_any "$HERE/missions/$TASK" "$DEST/missions"
else
    echo " banc d'instances: conserve en place (--avec-banc pour l'archiver aussi)"
    echo "                   ATTENTION : si tu as regenere missions/ depuis la campagne,"
    echo "                   le banc en place n'est PAS celui qui a servi. Les parametres"
    echo "                   reels de chaque mission restent lisibles dans les rapports."
fi

# --- INFO.txt : de quoi reconstituer les conditions plus tard ---------------
dire() {   # dire "<libelle>" <motif> <fichier>
    printf '  %-52s' "$1"
    if [ -f "$3" ] && grep -q "$2" "$3" 2>/dev/null; then echo "oui"; else echo "non"; fi
}

{
    echo "Campagnes archivees le $(date '+%Y-%m-%d %H:%M')"
    echo "Tache     : $TASK"
    echo "Campagnes : ${RETENUES[*]}"
    echo "Banc d'instances archive : $WITH_BENCH"
    echo
    echo "Pour retracer ces donnees :"
    for c in "${RETENUES[@]}"; do
        if [ "$c" = "courante" ]; then
            echo "  python3 plot_comparaison.py $TASK --archive $NAME"
        else
            echo "  python3 plot_comparaison.py $TASK --archive $NAME --${c//_/-}"
        fi
    done
    echo "  (ajoute --brut pour les valeurs brutes, sans minimum courant)"
    echo
    echo "Etat du depot au moment de l'archivage :"
    dire "patch distribution_radius applique" "LLM_MISSION_DISTRIBUTION_RATIO" \
         "$HERE/../software/argos3-installation/experiments-loop-functions/loop-functions/moca/SeqLoopFunc.cpp"
    for c in "${RETENUES[@]}"; do
        echo "  --- $c ---"
        case "$c" in
          courante)
            dire "prompt sans solutions (HOW TO APPROACH)" "HOW TO APPROACH THE DESIGN" \
                 "$HERE/automode_llm_common_missions.py"
            dire "graines communes (COMMON_RANDOM_NUMBERS)" "^COMMON_RANDOM_NUMBERS = True" \
                 "$HERE/run_paired_comparison_d_missions.py"
            grep -hE "^K_(BEST|WORST) = " "$HERE/run_paired_comparison_d_missions.py" 2>/dev/null \
                | sed 's/^/  retour au LLM : /'
            echo "  ABLATE au lancement : ${ABLATE:-aucune}"
            ;;
          origin)
            echo "  protocole d'origine : prompt initial, filtre de connectivite,"
            echo "  pre-filtrage a 3 graines NON decompte du budget"
            ;;
          complet)
            dire "retour au LLM : classement complet du round" "FULL RANKING" \
                 "$HERE/run_paired_comparaison_complet.py"
            echo "  instance : publiee, sans variation de parametres"
            ;;
          temoin|sans_*)
            grep -hE "^SANS_(CONTRAINTE_CONNECTIVITE|CLASSEMENT|CHAMPION) = " \
                 "$HERE/automode_sans_contrainte_ni_info.py" 2>/dev/null \
                | sed 's/^/  interrupteur : /'
            echo "  instance : publiee, sans variation de parametres"
            ;;
        esac
    done
} > "$DEST/INFO.txt"

echo
echo "$MOVED element(s) archives dans $DEST"
echo
cat "$DEST/INFO.txt"
echo
echo "Les campagnes archivees repartent de zero au prochain lancement."
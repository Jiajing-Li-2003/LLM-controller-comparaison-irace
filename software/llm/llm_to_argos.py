#!/usr/bin/env python3
# llm_to_argos.py

import subprocess
import sys
import re
import requests
import json

# ─── Contexte donné au LLM ────────────────────────────────────────────────────
SYSTEM_PROMPT = """
Tu es un traducteur strict vers le format FSM d'ARGoS3/AutoMoDe.
Réponds UNIQUEMENT avec la ligne "FSM:".

FORMAT OBLIGATOIRE :
FSM: --nstates N --s0 B0 [params0] [--n0 T --n0x0 D --c0x0 C --p0x0 P --w0x0 W] --s1 B1 [params1] [--n1 T --n1x0 D --c1x0 C --p1x0 P --w1x0 W]

COMPORTEMENTS (--sI BI) :
  0 = Exploration  → DOIT avoir --rwmI [1..100] ET --cleI [0..3]
  1 = Stop         → DOIT avoir --cleI [0..3]

COULEURS LED (--cleI) : 0=aucune 1=rouge 2=vert 3=bleu

TRANSITIONS (de l'état I vers état J) :
  --nI T         = nombre de transitions partant de l'état I (0 s'il n'y en a aucune)
  --nIx0 0       = destination (0 signifie l'autre état)
  --cIx0 TYPE    = type de condition : 
                   0 = sol noir
                   1 = sol gris
                   2 = sol blanc
                   3 = NbVoisins >= seuil
                   4 = NbVoisins < seuil
  --pIx0 P       = probabilité (toujours 1.0 par défaut)
  --wIx0 W       = LE SEUIL EXACT. Si l'utilisateur demande "4 voisins", alors W=4. N'utilise jamais ce nombre dans --cIx0 !

EXEMPLES CORRECTS :

Phrase : "les robots explorent en vert et s'arrêtent sur le sol noir en rouge"
FSM: --nstates 2 --s0 0 --rwm0 10 --cle0 2 --n0 1 --n0x0 0 --c0x0 0 --p0x0 1.0 --s1 1 --cle1 1 --n1 0

Phrase : "Les robots explorent en vert et s'arrêtent sur le sol noir en rouge. Si un robot détecte 4 voisins ou plus, il recommence à explorer."
FSM: --nstates 2 --s0 0 --rwm0 10 --cle0 2 --n0 1 --n0x0 0 --c0x0 0 --p0x0 1.0 --s1 1 --cle1 1 --n1 1 --n1x0 0 --c1x0 3 --p1x0 1.0 --w1x0 4
"""
# ─── Appel au LLM local (Ollama) ──────────────────────────────────────────────
def get_fsm_from_llm(description: str) -> str:
    prompt = SYSTEM_PROMPT + "\n\nUtilisateur: " + description

    print(f"\n📤 Envoi au LLM local : \"{description}\"")

    response = requests.post(
        "http://localhost:11434/api/generate",
        json={
            "model": "llama3.2",
            "prompt": prompt,
            "stream": False
        }
    )

    raw = response.json()["response"].strip()
    print(f"📥 Réponse brute : {raw}")

    match = re.search(r'FSM:\s*(.+)', raw)
    if not match:
        raise ValueError(f"Pas de ligne FSM: dans : {raw}")

    fsm_config = match.group(1).strip()
    print(f"✅ FSM extrait : {fsm_config}")
    return fsm_config

# ─── Mise à jour du fichier .argos ────────────────────────────────────────────
def update_argos_file(argos_path: str, fsm_config: str):
    with open(argos_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Regex très permissive — gère tous les cas
    new_content, count = re.subn(
        r'fsm-config\s*=\s*["\'][^"\']*["\']',
        f'fsm-config="{fsm_config}"',
        content
    )

    if count == 0:
        # Debug : affiche ce qui est dans le fichier
        for line in content.split('\n'):
            if 'fsm' in line.lower() or 'params' in line.lower():
                print(f"DEBUG ligne trouvée : {repr(line)}")
        raise ValueError("fsm-config non trouvé dans le fichier .argos")

    with open(argos_path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"📝 Fichier mis à jour : {argos_path}")

# ─── Lancement d'ARGoS ────────────────────────────────────────────────────────
def launch_argos(argos_path: str):
    print(f"\n🚀 Lancement de la simulation...")
    subprocess.run(["argos3", "-c", argos_path])

# ─── Programme principal ──────────────────────────────────────────────────────
def main():
    ARGOS_FILE = "/home/ubuntu/mandarina-supp/software/argos3-installation/" \
                 "experiments-loop-functions/scenarios/mandarina/" \
                 "automode_experiments/bxo-01-11.argos"

    # Phrase en argument ou en mode interactif
    if len(sys.argv) > 1:
        description = " ".join(sys.argv[1:])
    else:
        print("💬 Décris le comportement des robots :")
        description = input("> ").strip()

    if not description:
        print("❌ Description vide.")
        return

    fsm_config = get_fsm_from_llm(description)
    update_argos_file(ARGOS_FILE, fsm_config)
    launch_argos(ARGOS_FILE)

if __name__ == "__main__":
    main()

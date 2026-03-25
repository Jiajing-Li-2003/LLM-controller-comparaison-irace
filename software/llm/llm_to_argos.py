#!/usr/bin/env python3
# llm_to_argos.py

import subprocess
import sys
import re
import requests
import json

# ─── Contexte donné au LLM ────────────────────────────────────────────────────
SYSTEM_PROMPT = """
Tu es un traducteur strict de texte vers une configuration FSM pour ARGoS3.

RÈGLES ABSOLUES :
1. Réponds UNIQUEMENT avec la ligne commençant par "FSM:".
2. L'état Stop (--sX 1) ne prend QUE le paramètre --cleX. N'ajoute JAMAIS --rwmX à un état Stop.

CORRESPONDANCES DES TRANSITIONS (--nIxJ) :
- "sol noir" = 0
- "sol gris" = 1
- "sol blanc" = 2

EXEMPLES PARFAITS (à imiter scrupuleusement) :

Phrase : "les robots explorent et s'arrêtent sur le sol noir"
FSM: --nstates 2 --s0 0 --rwm0 10 --cle0 0 --s1 1 --cle1 0 --n0x1 0 --p0x1 1.0

Phrase : "les robots explorent et s'arrêtent sur le sol blanc"
FSM: --nstates 2 --s0 0 --rwm0 10 --cle0 0 --s1 1 --cle1 0 --n0x1 2 --p0x1 1.0
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

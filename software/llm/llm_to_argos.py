#!/usr/bin/env python3
# llm_to_argos.py

import subprocess
import sys
import re
import requests
import json

# ─── Context provided to the LLM ──────────────────────────────────────────────
SYSTEM_PROMPT = """
You are an expert translator to the ARGoS3/AutoMoDe FSM format.

STEP 1: THINKING
Analyze the request and write down the states and transitions required.

STEP 2: FSM GENERATION
Output the final configuration starting strictly with "FSM: " on a single line.

MANDATORY FORMAT:
FSM: --nstates N --s0 B [params0] [--n0 T --n0x0 D --c0x0 C --p0x0 P --w0x0 W] --s1 B [params1] [--n1 T --n1x0 D --c1x0 C --p1x0 P --w1x0 W]

═══ BEHAVIORS (--sI B) ═══
  0 = Exploration    → MANDATORY params: --rwmI [1..100]  --cleI [0..3]
  1 = Stop           → MANDATORY params: --cleI [0..3]
  2 = Phototaxis     → MANDATORY params: --cleI [0..3]
  3 = AntiPhototaxis → MANDATORY params: --cleI [0..3]
  4 = Attraction     → MANDATORY params: --attI [1..10]   --cleI [0..3]
  5 = Repulsion      → MANDATORY params: --repI [1..10]   --cleI [0..3]

═══ LED COLORS (--cleI) ═══
  0=none  1=red  2=green  3=blue

═══ TRANSITIONS (from state I to state J) ═══
  --nI T       = number of transitions leaving state I (0 = none)
  --nIxK D     = destination index (0 = 1st other state, 1 = 2nd other state)
  --cIxK C     = condition type:
                 0=black floor   1=gray floor   2=white floor
                 3=NeighborsCount >= W   4=NeighborsCount < W
  --pIxK P     = trigger probability (always 1.0)
  --wIxK W     = NEIGHBORS THRESHOLD — MANDATORY if --cIxK is 3 or 4

═══ CORRECT EXAMPLES ═══

Phrase: "The robots explore in green. If they see the black floor, they stop in red. Then, if a robot detects 3 neighbors or more, it moves away in blue."
Thinking:
- 3 states needed.
- State 0: Exploration (green). 1 transition: black floor -> State 1.
- State 1: Stop (red). 1 transition: NeighborsCount >= 3 -> State 2. (requires --w1x0 3)
- State 2: Repulsion (blue). 0 transitions.
FSM: --nstates 3 --s0 0 --rwm0 10 --cle0 2 --n0 1 --n0x0 0 --c0x0 0 --p0x0 1.0 --s1 1 --cle1 1 --n1 1 --n1x0 1 --c1x0 3 --p1x0 1.0 --w1x0 3 --s2 5 --rep2 5 --cle2 3 --n2 0
"""
# ─── Local LLM Call (Ollama) ────────────────────────────────────────────────
def get_fsm_from_llm(description: str) -> str:
    prompt = SYSTEM_PROMPT + "\n\nUser: " + description

    print(f"\n📤 Sending to local LLM: \"{description}\"")

    response = requests.post(
        "http://localhost:11434/api/generate",
        json={
            "model": "mistral",
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": 0.0
            }
        }
    )

    raw = response.json()["response"].strip()
    print(f"📥 Raw response:\n{raw}")

    # Vérifie si le marqueur FSM: est bien là
    if "FSM:" not in raw:
        raise ValueError(f"No FSM: marker found in: {raw}")

    # Coupe le texte pour ne garder que ce qui est APRES "FSM:"
    fsm_part = raw.split("FSM:")[1]

    # Nettoie les balises markdown (```) si l'IA en a mis
    fsm_part = fsm_part.replace("```", "")

    # Remplace tous les sauts de ligne par des espaces et nettoie les blancs
    fsm_config = " ".join(fsm_part.split())

    if not fsm_config:
        raise ValueError("L'extraction de la FSM a donné un résultat vide.")

    print(f"✅ Extracted FSM: {fsm_config}")
    return fsm_config
# ─── Updating the .argos file ───────────────────────────────────────────────
def update_argos_file(argos_path: str, fsm_config: str):
    with open(argos_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Very permissive Regex — handles all cases
    new_content, count = re.subn(
        r'fsm-config\s*=\s*["\'][^"\']*["\']',
        f'fsm-config="{fsm_config}"',
        content
    )

    if count == 0:
        # Debug: prints what is found in the file
        for line in content.split('\n'):
            if 'fsm' in line.lower() or 'params' in line.lower():
                print(f"DEBUG line found: {repr(line)}")
        raise ValueError("fsm-config not found in the .argos file")

    with open(argos_path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"📝 File updated: {argos_path}")

# ─── Launching ARGoS ────────────────────────────────────────────────────────
def launch_argos(argos_path: str):
    print(f"\n🚀 Launching the simulation...")
    subprocess.run(["argos3", "-c", argos_path])

# ─── Main Program ───────────────────────────────────────────────────────────
def main():
    ARGOS_FILE = "/home/ubuntu/mandarina-supp/software/argos3-installation/" \
                 "experiments-loop-functions/scenarios/mandarina/" \
                 "automode_experiments/bxo-01-11.argos"

    # Phrase as argument or in interactive mode
    if len(sys.argv) > 1:
        description = " ".join(sys.argv[1:])
    else:
        print("💬 Describe the robots' behavior:")
        description = input("> ").strip()

    if not description:
        print("❌ Empty description.")
        return

    fsm_config = get_fsm_from_llm(description)
    update_argos_file(ARGOS_FILE, fsm_config)
    launch_argos(ARGOS_FILE)

if __name__ == "__main__":
    main()
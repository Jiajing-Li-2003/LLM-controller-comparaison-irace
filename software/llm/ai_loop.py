#!/usr/bin/env python3
# ai_loop.py

import subprocess
import sys
import re
import time
import threading
import queue
from flask import Flask, request, jsonify
from google import genai
from google.genai import types
import logging

# ─── Configuration ────────────────────────────────────────────────────────────
API_KEY = "AIzaSyA0SUKBI0quY1Grrgdu-cm8oT7bF7RsTck"
ARGOS_FILE = "/home/ubuntu/mandarina-supp/software/argos3-installation/" \
             "experiments-loop-functions/scenarios/mandarina/" \
             "automode_experiments/bxo-01-11.argos"

# File d'attente pour passer le score du serveur web au script principal
score_queue = queue.Queue()

# ─── Serveur Flask (Tourne en tâche de fond) ──────────────────────────────────
app = Flask(__name__)
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR) # Cache les logs polluants de Flask

@app.route('/update_goal', methods=['POST'])
def receive_score():
    data = request.get_json()
    if data and 'current_score' in data:
        score = data['current_score']
        score_queue.put(score) # Envoie le score au script principal
    return jsonify({"message": "ok"}), 200

def run_flask():
    app.run(host='0.0.0.0', port=5000, use_reloader=False)

# ─── Prompt Système ───────────────────────────────────────────────────────────
SYSTEM_PROMPT = """
You are an expert AI optimizing an ARGoS3/AutoMoDe swarm robotics configuration.
You will generate FSMs to solve a specific task and maximize the score. 
The user will feed you back your score. Use your Thinking phase to analyze why the previous FSM failed or succeeded, and propose a NEW strategy.

STEP 1: THINKING
Analyze the goal, the previous score, and design a better behavior.

STEP 2: FSM GENERATION
Output the final configuration starting strictly with "FSM: " on a single line.

MANDATORY FORMAT:
FSM: --nstates N --s0 B [params0] [--n0 T --n0x0 D --c0x0 C --p0x0 P --w0x0 W] ...

═══ BEHAVIORS (--sI B) ═══
  0 = Exploration    (req: --rwmI [1..100]  --cleI [0..3])
  1 = Stop           (req: --cleI [0..3])
  2 = Phototaxis     (req: --cleI [0..3])
  3 = AntiPhototaxis (req: --cleI [0..3])
  4 = Attraction     (req: --attI [1..10]   --cleI [0..3])
  5 = Repulsion      (req: --repI [1..10]   --cleI [0..3])

═══ LED COLORS (--cleI) ═══
  0=none  1=red  2=green  3=blue

═══ TRANSITIONS (from state I to state J) ═══
  --nI T       = number of transitions leaving state I (0 = none)
  --nIxK D     = destination index (0-indexed among REMAINING states)
  --cIxK C     = condition type (0=black floor, 1=gray, 2=white, 3=NeighborsCount >= W, 4=NeighborsCount < W)
  --pIxK P     = trigger probability (always 1.0)
  --wIxK W     = NEIGHBORS THRESHOLD (MANDATORY if --cIxK is 3 or 4)
"""

# ─── Fonctions Utilitaires ────────────────────────────────────────────────────
def update_argos_file(fsm_config: str):
    with open(ARGOS_FILE, 'r', encoding='utf-8') as f:
        content = f.read()
    new_content, count = re.subn(r'fsm-config\s*=\s*["\'][^"\']*["\']', f'fsm-config="{fsm_config}"', content)
    if count > 0:
        with open(ARGOS_FILE, 'w', encoding='utf-8') as f:
            f.write(new_content)

def extract_fsm(raw_text: str) -> str:
    if "FSM:" not in raw_text:
        return ""
    fsm_part = raw_text.split("FSM:")[1].replace("```", "")
    return " ".join(fsm_part.split())

# ─── Boucle d'Apprentissage Autonome ──────────────────────────────────────────
def main():
    print("🌐 Démarrage du serveur des scores en arrière-plan...")
    threading.Thread(target=run_flask, daemon=True).start()
    time.sleep(1) # Laisse le temps au serveur de démarrer

    client = genai.Client(api_key=API_KEY)
    
    # On initialise un CHAT (Gemini se souviendra de la conversation)
    chat = client.chats.create(
        model='gemini-2.5-flash',
        config=types.GenerateContentConfig(
            system_instruction=SYSTEM_PROMPT,
            temperature=0.3 # Légère créativité pour qu'il teste de nouvelles choses
        )
    )

    print("\n💬 Quel est l'objectif global que les robots doivent apprendre ?")
    print("(ex: 'Maximize the Manufacture score by stopping on the white floor')")
    goal = input("> ").strip()
    if not goal: return

    iterations = 5 # Nombre d'essais que Gemini a pour trouver la solution
    last_score = 0.0

    for i in range(iterations):
        print(f"\n" + "="*50)
        print(f"🔄 ITERATION {i+1} / {iterations}")
        print("="*50)

        # Construction du message pour Gemini
        if i == 0:
            prompt = f"Target goal: {goal}. Provide your first FSM attempt."
        else:
            prompt = f"Your last FSM got a score of {last_score}. Please analyze what might have gone wrong, and provide a NEW, different FSM strategy to increase the score."

        print("🧠 Gemini réfléchit à une stratégie...")
        response = chat.send_message(prompt)
        raw_text = response.text.strip()
        
        # Affiche ce que Gemini a pensé (utile pour comprendre sa logique)
        print(f"💡 Logique de l'IA :\n{raw_text.split('FSM:')[0].strip()}\n")

        fsm_config = extract_fsm(raw_text)
        if not fsm_config:
            print("❌ Gemini n'a pas généré de FSM valide. On passe au prochain essai.")
            continue

        print(f"✅ FSM Générée : {fsm_config}")
        update_argos_file(fsm_config)

        print("🚀 Lancement express d'ARGoS3 (Mode Headless - Ultra rapide)...")
        # Le "-z" lance la simulation SANS interface visuelle (calcul pur)
        subprocess.run(["argos3", "-c", ARGOS_FILE, "-z"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        try:
            # On attend que le C++ envoie le score au serveur
            last_score = score_queue.get(timeout=5)
            print(f"🏆 SCORE OBTENU : {last_score}")
        except queue.Empty:
            print("⚠️ Aucun score reçu du C++. Score défini à 0.0")
            last_score = 0.0

    print("\n🎉 Fin de la boucle d'apprentissage ! Le meilleur comportement a été trouvé.")

if __name__ == "__main__":
    main()
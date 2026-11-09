#!/usr/bin/env python3
# server_score.py

from flask import Flask, request, jsonify

app = Flask(__name__)

# ─── Endpoint correspondant à SeqLoopFunc.cpp ─────────────────────────────────
@app.route('/update_goal', methods=['POST'])
def receive_score():
    try:
        # Récupère les données JSON envoyées par ARGoS3
        data = request.get_json()
        
        if not data or 'current_score' not in data:
            return jsonify({"error": "No current_score found in the request"}), 400

        score = data['current_score']
        blue_task = data.get('blue_task', 'N/A')
        red_task = data.get('red_task', 'N/A')
        
        # Affiche le score en grand dans le terminal !
        print("\n" + "⭐"*20)
        print(f"🏆 SCORE REÇU : {score}")
        print(f"🟦 Tâche Bleue : {blue_task} | 🟥 Tâche Rouge : {red_task}")
        print("⭐"*20 + "\n")

        # Le C++ s'attend à recevoir une réponse JSON qui peut contenir de nouvelles tâches
        # Pour l'instant, on renvoie juste "ok"
        response_data = {
            "message": "Score bien reçu par le serveur Python !",
            # Décommenter les lignes ci-dessous on veux que l'IA change les tâches :
            # "new_blue_task": 1,
            # "new_red_task": 2
        }

        return jsonify(response_data), 200

    except Exception as e:
        print(f"❌ Erreur lors de la réception : {e}")
        return jsonify({"error": str(e)}), 500

# ─── Lancement du serveur ───────────────────────────────────────────────────
if __name__ == '__main__':
    print("🚀 Serveur Flask en écoute sur le port 5000...")
    print("En attente de la fin de la simulation ARGoS3...")
    app.run(host='0.0.0.0', port=5000)
from flask import Flask, request, jsonify
from google import genai
import json

app = Flask(__name__)

# --- CONFIGURATION ---
API_KEY = "AIzaSyA0SUKBI0quY1Grrgdu-cm8oT7bF7RsTck" 
client = genai.Client(api_key=API_KEY)

def get_working_model():
    """Liste les modèles et choisit le premier qui ressemble à 'flash'."""
    print("[SYSTEM] Diagnostic des modèles disponibles...")
    try:
        available_models = list(client.models.list())
        for m in available_models:
            print(f"  - Trouvé : {m.name}")
        
        # On cherche un modèle flash dans la liste
        for target in ["gemini-1.5-flash", "gemini-2.0-flash", "flash"]:
            for m in available_models:
                if target in m.name:
                    return m.name
        return "models/gemini-1.5-flash" # Fallback
    except Exception as e:
        print(f"[ERREUR DIAGNOSTIC] : {e}")
        return "models/gemini-1.5-flash"

MODEL_ID = get_working_model()
print(f"[SYSTEM] Modèle sélectionné : {MODEL_ID}")

@app.route('/update_goal', methods=['POST'])
def update_goal():
    print("\n" + "="*40)
    print("[INFO] Simulation terminée. Réception des données...")
    data = request.get_json(force=True)
    score = data.get('current_score', 0.0)
    
    prompt = f"Score simulation : {score}. Choisis 2 IDs de tâches (0-5). Réponds uniquement en JSON : {{\"message\": \"...\", \"new_blue\": 0, \"new_red\": 0}}"

    try:
        print(f"[IA] Appel de {MODEL_ID}...")
        response = client.models.generate_content(model=MODEL_ID, contents=prompt)
        
        # Extraction du JSON
        txt = response.text
        start, end = txt.find('{'), txt.rfind('}') + 1
        decision = json.loads(txt[start:end])
        
        print(f"[IA] Succès ! Message : {decision['message']}")
        return jsonify({
            "message": decision['message'],
            "new_blue_task": int(decision['new_blue']),
            "new_red_task": int(decision['new_red']),
            "new_eval_task": 0
        })

    except Exception as e:
        print(f"[ERREUR IA] : {e}")
        print("\n--- MODE MANUEL (SÉCURISÉ) ---")
        print("ATTENTION : Ne pas utiliser les flèches du clavier !")
        
        # Boucle pour éviter que le serveur crash si tu te trompes de touche
        while True:
            try:
                nb = int(input("Entrez ID Bleu (0-5) : "))
                nr = int(input("Entrez ID Rouge (0-5) : "))
                break 
            except ValueError:
                print("Erreur : Tapez uniquement des CHIFFRES (0, 1, 2, 3, 4 ou 5).")
        
        return jsonify({"message": "Repli manuel", "new_blue_task": nb, "new_red_task": nr, "new_eval_task": 0})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)

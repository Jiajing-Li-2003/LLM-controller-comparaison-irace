#include "SeqLoopFunc.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

/****************************************/
/****************************************/

SeqLoopFunction::SeqLoopFunction() {
    m_unClock = 0;
    m_fObjectiveFunction = 0;
    m_fObjectiveFunctionBlue = 0;
    m_fObjectiveFunctionRed = 0;
    m_bBlueFirst = true;
    m_cArenaColor = CColor::BLACK;
    m_cTaskAsignedColorBlue = CColor::BLACK;
    m_cTaskEvalColorBlue = CColor::BLACK;
    m_cTaskAsignedColorRed = CColor::BLACK;
    m_cTaskEvalColorRed = CColor::BLACK;
    m_vArenaState.resize(9, 0);
    m_vArenaStatePast.resize(9, 0);
    m_sBaseCommand = "ssh moca@192.168.1.149 < /home/trackingsystem/dgarzon/sw-mandarina/moca-scripts/commands/";
    m_sSufixCommand = ".txt &";
}

SeqLoopFunction::SeqLoopFunction(const SeqLoopFunction& orig) {}

SeqLoopFunction::~SeqLoopFunction() {}

void SeqLoopFunction::Destroy() {
    m_tRobotStates.clear();
    m_tSourceItems.clear();
    m_tSourceOperation.clear();
    m_tSourceReparation.clear();
    m_tSourceRestoring.clear();
    m_tArenaPoints.clear();
    m_vArenaState.clear();
    m_vArenaStatePast.clear();
    RemoveArena();
}

void SeqLoopFunction::Init(TConfigurationNode& t_tree) {
    CoreLoopFunctions::Init(t_tree);
    TConfigurationNode cParametersNode;
    try {
        cParametersNode = GetNode(t_tree, "params");
        GetNodeAttributeOrDefault(cParametersNode, "number_robots", m_unNumberRobots, (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "build_arena", m_bBuildArena, (bool) false);
        GetNodeAttributeOrDefault(cParametersNode, "number_edges", m_unNumberEdges, (UInt32) 3);
        GetNodeAttributeOrDefault(cParametersNode, "number_boxes_per_edge", m_unNumberBoxes, (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "lenght_boxes", m_fLenghtBoxes, (Real) 0.25);
        GetNodeAttributeOrDefault(cParametersNode, "color_order", m_unColorOrder, (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "number_colors", m_unNumerColors, (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "blue_task", m_unBlueTask, (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "red_task", m_unRedTask, (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "eval_task", m_unEvalTask, (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "trn_time", m_unTrnTime, (UInt32) 600);
        GetNodeAttributeOrDefault(cParametersNode, "alpha_value", m_fAlphaValue, (Real) 0.5);
        GetNodeAttributeOrDefault(cParametersNode, "real_experiments", m_bRealExperiments, (bool) false);
    } catch(std::exception& e) {}

    if (m_bBuildArena == true) {
        m_fDistributionRadius = GetArenaRadious();
        PositionArena();
    }
    m_bBlueFirst = SelectColorOrder(m_unColorOrder);
    AsignArenaColors(m_unNumerColors);
    InitRobotStates();
    InitSources();
    ArenaControl();
}

void SeqLoopFunction::Reset() {
    CoreLoopFunctions::Reset();
    m_unClock = 0;
    m_fObjectiveFunction = 0;
    m_fObjectiveFunctionBlue = 0;
    m_fObjectiveFunctionRed = 0;
    m_bBlueFirst = SelectColorOrder(m_unColorOrder);
    m_tRobotStates.clear();
    m_tSourceItems.clear();
    m_tSourceOperation.clear();
    m_tSourceReparation.clear();
    m_tSourceRestoring.clear();
    m_tArenaPoints.clear();
    InitRobotStates();
    InitSources();
    std::fill(m_vArenaStatePast.begin(), m_vArenaStatePast.end(), 0);
    std::fill(m_vArenaState.begin(), m_vArenaState.end(), 0);
    ArenaControl();
}

void SeqLoopFunction::PostStep() {
    m_unClock = GetSpace().GetSimulationClock();
    ScoreControl();
    ArenaControl();
}

/******************* INTERACTION API LLM *********************/

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void SeqLoopFunction::CommunicateWithLLM(float score) {
    CURL* curl = curl_easy_init();
    if(curl) {
        std::string readBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/update_goal");
        
        // 1. Préparation du JSON avec les vraies variables
        json payload;
        payload["current_score"] = score;
        payload["blue_task"] = m_unBlueTask;
        payload["red_task"] = m_unRedTask;
        payload["eval_task"] = m_unEvalTask;
        std::string data = payload.dump();

        // 2. Ajout des Headers (Content-Type) pour Flask
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // 3. Configuration de l'envoi
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        std::cout << "\n[API] Envoi des données au serveur Python..." << std::endl;

        // 4. Exécution de la requête
        if(curl_easy_perform(curl) == CURLE_OK) {
            try {
                auto response = json::parse(readBuffer);
                std::cout << "[LLM] Message reçu : " << response.value("message", "Ok") << std::endl;
                
                // Mise à jour des objectifs selon le retour du serveur
                if (response.contains("new_blue_task")) m_unBlueTask = response["new_blue_task"];
                if (response.contains("new_red_task")) m_unRedTask = response["new_red_task"];
                
            } catch (std::exception& e) {
                std::cerr << "[API] Erreur lecture JSON : " << e.what() << std::endl;
            }
        } else {
            std::cerr << "[API] Échec : Serveur Python injoignable (port 5000)." << std::endl;
        }

        // Nettoyage
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
void SeqLoopFunction::PostExperiment() {
    if (m_unEvalTask == 0) {
        m_fObjectiveFunction = (m_fAlphaValue)*GetNormalizedScore(m_fObjectiveFunctionBlue, m_unBlueTask)
                              +(1-m_fAlphaValue)*GetNormalizedScore(m_fObjectiveFunctionRed, m_unRedTask);
    } else if (m_unEvalTask == 1) m_fObjectiveFunction = m_fObjectiveFunctionBlue;
      else if (m_unEvalTask == 2) m_fObjectiveFunction = m_fObjectiveFunctionRed;

    LOG << "Score Final: " << m_fObjectiveFunction << std::endl;
    CommunicateWithLLM(m_fObjectiveFunction);
}

Real SeqLoopFunction::GetObjectiveFunction() {
    if (m_unEvalTask == 0) return m_fObjectiveFunction;
    if (m_unEvalTask == 1) return m_fObjectiveFunctionBlue;
    if (m_unEvalTask == 2) return m_fObjectiveFunctionRed;
    return 0;
}

/****************************************/
/* IMPLEMENTATION DES FONCTIONS CRITIQUES */
/****************************************/

// Correction de l'erreur "GetFloorColor"
CColor SeqLoopFunction::GetFloorColor(const CVector2& c_pos) {
    if (c_pos.GetX() >= -0.125 && c_pos.GetX() <= 0.125){
        if (c_pos.GetY() >= 0.65033 || c_pos.GetY() <= -0.65033) return CColor::BLACK;
        else if (c_pos.GetY() >= -0.125 && c_pos.GetY() <= 0.125) return CColor::WHITE;
    }
    else if (c_pos.GetY() >= -0.125 && c_pos.GetY() <= 0.125){
        if (c_pos.GetX() >= 0.65033 || c_pos.GetX() <= -0.65033) return CColor::BLACK;
    }
    return CColor::GRAY50;
}

CVector3 SeqLoopFunction::GetRandomPosition() {
    Real a = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
    Real b = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
    Real c = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
    Real d = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
    if (b < a) std::swap(a, b);
    Real fRad = 0.4;
    Real fPosX = (c * fRad / 2.0) + fRad * cos(2.0 * CRadians::PI.GetValue() * (a/b));
    Real fPosY = (d * fRad / 2.0) + fRad * sin(2.0 * CRadians::PI.GetValue() * (a/b));
    return CVector3(fPosX, fPosY, 0);
}

UInt32 SeqLoopFunction::GetRandomTime(UInt32 min, UInt32 max) {
    return m_pcRng->Uniform(CRange<UInt32>(min, max));
}

void SeqLoopFunction::PositionArena() {
    m_pcArena = new CArenaEntity("arena", CVector3(0,0,0), CQuaternion(), CVector3(0.01, m_fLenghtBoxes, 0.1), "leds", m_unNumberBoxes, m_unNumberEdges, 0.125f, 1.0f);
    AddEntity(*m_pcArena);
}

void SeqLoopFunction::RemoveArena() {
    try { RemoveEntity("arena"); } catch(...) {}
}

Real SeqLoopFunction::GetArenaRadious() {
    return (m_fLenghtBoxes * m_unNumberBoxes) / (2.0 * Tan(CRadians::PI / m_unNumberEdges)) - 0.65;
}

/****************************************/
/* LOGIQUE DE SCORE ET CAPTEURS */
/****************************************/

void SeqLoopFunction::ArenaControl() {
    if (m_unClock == 0) m_cArenaColor = m_bBlueFirst ? m_cTaskAsignedColorBlue : m_cTaskAsignedColorRed;
    if (m_unClock == m_unTrnTime) m_cArenaColor = m_bBlueFirst ? m_cTaskAsignedColorRed : m_cTaskAsignedColorBlue;
    
    if (m_unClock == 0 || m_unClock == m_unTrnTime) {
        m_pcArena->SetArenaColor(m_cArenaColor);
        for (UInt32 i=1; i<9; ++i) {
            bool active = m_tSourceOperation[i] > m_unClock;
            m_vArenaState[i] = active ? 1 : 0;
            m_pcArena->SetBoxColor(2, i, active ? CColor::GREEN : CColor::BLACK);
        }
    }
}

void SeqLoopFunction::ScoreControl() {
    if ((m_bBlueFirst && m_unClock <= m_unTrnTime) || (!m_bBlueFirst && m_unClock > m_unTrnTime))
        m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
    else
        m_fObjectiveFunctionRed += GetScore(m_unRedTask);
}

Real SeqLoopFunction::GetScore(UInt32 task) {
    if (task == 0) return -GetManufactureScore();
    if (task == 1) return -GetTransportScore();
    if (task == 5) return -GetRechargeScore();
    return 0;
}

Real SeqLoopFunction::GetNormalizedScore(Real score, UInt32 task) { return score; }

Real SeqLoopFunction::GetRechargeScore() {
    UpdateRobotPositions(); Real s=0;
    for (auto it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) 
        if (IsRobotInDock(it->second.cPosition)) s++;
    return s;
}

Real SeqLoopFunction::GetTransportScore() {
    UpdateRobotPositions(); Real s=0;
    for (auto it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        if (it->second.bItem) { if (IsRobotInNest(it->second.cPosition)) { s++; it->second.bItem=false; } }
        else { if (IsRobotInSourceID(it->second.cPosition)!=0) it->second.bItem=true; }
    }
    return s;
}

Real SeqLoopFunction::GetManufactureScore() {
    UpdateRobotPositions(); Real s=0;
    for (auto it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        UInt32 id = IsRobotInSourceID(it->second.cPosition);
        if (id != 0 && m_tSourceOperation[id] > m_unClock) {
            if (++m_tSourceItems[id] >= 100) { m_tSourceItems[id]=0; s++; }
        }
    }
    return s;
}

bool SeqLoopFunction::IsRobotInNest(CVector2 p) { return (abs(p.GetX()) < 0.16 && abs(p.GetY()) < 0.16); }

bool SeqLoopFunction::IsRobotInDock(CVector2 p) { return (p.Length() > 0.65); }

UInt32 SeqLoopFunction::IsRobotInSourceID(CVector2 p) {
    if (p.Length() < 0.6) return 0;
    if (abs(p.GetX()) < 0.16) return (p.GetY() > 0) ? 2 : 6;
    if (abs(p.GetY()) < 0.16) return (p.GetX() > 0) ? 8 : 4;
    return 0;
}

void SeqLoopFunction::UpdateRobotPositions() {
    CSpace::TMapPerType& m = GetSpace().GetEntitiesByType("epuck");
    for (auto it = m.begin(); it != m.end(); ++it) {
        CEPuckEntity* e = any_cast<CEPuckEntity*>(it->second);
        m_tRobotStates[e].cPosition.Set(e->GetEmbodiedEntity().GetOriginAnchor().Position.GetX(), e->GetEmbodiedEntity().GetOriginAnchor().Position.GetY());
    }
}

void SeqLoopFunction::InitRobotStates() {
    CSpace::TMapPerType& m = GetSpace().GetEntitiesByType("epuck");
    for (auto it = m.begin(); it != m.end(); ++it) {
        CEPuckEntity* e = any_cast<CEPuckEntity*>(it->second);
        m_tRobotStates[e].bItem = false;
    }
}

void SeqLoopFunction::InitSources() {
    for (int i=1; i<9; i++) { m_tSourceItems[i]=0; m_tSourceOperation[i]=GetRandomTime(1, 1200); }
}

bool SeqLoopFunction::SelectColorOrder(UInt32 v) { return v > 0; }

void SeqLoopFunction::AsignArenaColors(UInt32 v) {
    m_cTaskAsignedColorBlue = CColor::BLUE; m_cTaskAsignedColorRed = CColor::RED;
}

REGISTER_LOOP_FUNCTIONS(SeqLoopFunction, "seq_loop_function");

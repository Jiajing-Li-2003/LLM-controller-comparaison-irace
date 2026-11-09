/**
  * @file <loop-functions/IcraLoopFunc.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @license MIT License
  *
  * @modification : Ajout de l'intégration API LLM via libcurl + nlohmann/json
  */

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

/****************************************/
/****************************************/

SeqLoopFunction::SeqLoopFunction(const SeqLoopFunction& orig) {
}

/****************************************/
/****************************************/

SeqLoopFunction::~SeqLoopFunction() {}

/****************************************/
/****************************************/

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

/****************************************/
/****************************************/

void SeqLoopFunction::Init(TConfigurationNode& t_tree) {
    CoreLoopFunctions::Init(t_tree);
    TConfigurationNode cParametersNode;
    try {
        cParametersNode = GetNode(t_tree, "params");
        GetNodeAttributeOrDefault(cParametersNode, "number_robots",        m_unNumberRobots,    (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "build_arena",          m_bBuildArena,       (bool)   false);
        GetNodeAttributeOrDefault(cParametersNode, "number_edges",         m_unNumberEdges,     (UInt32) 3);
        GetNodeAttributeOrDefault(cParametersNode, "number_boxes_per_edge",m_unNumberBoxes,     (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "lenght_boxes",         m_fLenghtBoxes,      (Real)   0.25);
        // LLM_MISSION_DISTRIBUTION_RATIO : fraction du rayon interne de l'arene
        GetNodeAttributeOrDefault(cParametersNode, "distribution_radius_ratio", m_fDistributionRatio, (Real) 0.45);
        GetNodeAttributeOrDefault(cParametersNode, "color_order",          m_unColorOrder,      (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "number_colors",        m_unNumerColors,     (UInt32) 1);
        GetNodeAttributeOrDefault(cParametersNode, "t",            m_unBlueTask,        (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "red_task",             m_unRedTask,         (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "eval_task",            m_unEvalTask,        (UInt32) 0);
        GetNodeAttributeOrDefault(cParametersNode, "trn_time",             m_unTrnTime,         (UInt32) 600);
        GetNodeAttributeOrDefault(cParametersNode, "alpha_value",          m_fAlphaValue,       (Real)   0.5);
        GetNodeAttributeOrDefault(cParametersNode, "real_experiments",     m_bRealExperiments,  (bool)   false);
    } catch(std::exception e) {}

    if (m_bBuildArena == true) {
        m_fDistributionRadius = GetArenaRadious();
        PositionArena();
    }

    if (m_bRealExperiments) {
        std::string sTmpCommand = m_sBaseCommand + "C200000000" + m_sSufixCommand;
        // system(sTmpCommand.c_str());
    }

    m_bBlueFirst = SelectColorOrder(m_unColorOrder);
    AsignArenaColors(m_unNumerColors);
    InitRobotStates();
    InitSources();
    ArenaControl();
}

/****************************************/
/****************************************/

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

/****************************************/
/****************************************/

void SeqLoopFunction::PostStep() {
    m_unClock = GetSpace().GetSimulationClock();
    ScoreControl();
    ArenaControl();
}

/****************************************/
/****************************************/

void SeqLoopFunction::PostExperiment() {

    if (m_unEvalTask == 0) {
        m_fObjectiveFunction = (m_fAlphaValue) * GetNormalizedScore(m_fObjectiveFunctionBlue, m_unBlueTask)
                             + (1 - m_fAlphaValue) * GetNormalizedScore(m_fObjectiveFunctionRed, m_unRedTask);
        LOG << m_fObjectiveFunction << std::endl;
    }
    else if (m_unEvalTask == 1)
        LOG << m_fObjectiveFunctionBlue << std::endl;
    else if (m_unEvalTask == 2)
        LOG << m_fObjectiveFunctionRed << std::endl;
    else if (m_unEvalTask == 3) {
        LOG << "BlueObjective " << m_fObjectiveFunctionBlue << std::endl;
        LOG << "RedObjective "  << m_fObjectiveFunctionRed  << std::endl;
    }
    else
        LOG << 888888 << std::endl;

    // ─── Envoi du score final au serveur LLM ──────────────────────────────────
    CommunicateWithLLM(m_fObjectiveFunction);
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetObjectiveFunction() {
    if (m_unEvalTask == 0) return m_fObjectiveFunction;
    if (m_unEvalTask == 1) return m_fObjectiveFunctionBlue;
    if (m_unEvalTask == 2) return m_fObjectiveFunctionRed;
    if (m_unEvalTask == 3) return 999999;
    return 888888;
}

/****************************************/
/* INTERACTION API LLM                  */
/****************************************/

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void SeqLoopFunction::CommunicateWithLLM(float score) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[API] Erreur : impossible d'initialiser curl." << std::endl;
        return;
    }

    std::string readBuffer;

    // Construction du payload JSON
    json payload;
    payload["current_score"] = score;
    payload["blue_task"]     = m_unBlueTask;
    payload["red_task"]      = m_unRedTask;
    payload["eval_task"]     = m_unEvalTask;
    payload["clock"]         = m_unClock;
    std::string data = payload.dump();

    // Headers HTTP
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configuration de la requête
    curl_easy_setopt(curl, CURLOPT_URL,           "http://localhost:5000/update_goal");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,       10L); // timeout 10 secondes

    std::cout << "\n[API] Envoi score=" << score
              << " blue_task=" << m_unBlueTask
              << " red_task="  << m_unRedTask  << std::endl;

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        try {
            auto response = json::parse(readBuffer);
            std::cout << "[LLM] Réponse : " << response.value("message", "ok") << std::endl;

            // Mise à jour des tâches si le LLM en suggère de nouvelles
            if (response.contains("new_blue_task")) {
                m_unBlueTask = response["new_blue_task"];
                std::cout << "[LLM] Nouvelle blue_task : " << m_unBlueTask << std::endl;
            }
            if (response.contains("new_red_task")) {
                m_unRedTask = response["new_red_task"];
                std::cout << "[LLM] Nouvelle red_task : " << m_unRedTask << std::endl;
            }
        } catch (std::exception& e) {
            std::cerr << "[API] Erreur lecture JSON : " << e.what() << std::endl;
        }
    } else {
        std::cerr << "[API] Serveur Python injoignable (port 5000). "
                  << "Erreur curl : " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

/****************************************/
/****************************************/

void SeqLoopFunction::ArenaControl() {

    if (m_unClock == 0) {
        if (m_bBlueFirst)
            m_cArenaColor = m_cTaskAsignedColorBlue;
        else
            m_cArenaColor = m_cTaskAsignedColorRed;
    }

    if (m_unClock == m_unTrnTime) {
        if (m_bBlueFirst)
            m_cArenaColor = m_cTaskAsignedColorRed;
        else
            m_cArenaColor = m_cTaskAsignedColorBlue;
    }

    if (m_unClock == 0 || m_unClock == m_unTrnTime) {
        m_pcArena->SetArenaColor(m_cArenaColor);

        if (m_cArenaColor == CColor::BLUE)
            m_vArenaState[0] = 0;
        else
            m_vArenaState[0] = 1;

        for (UInt32 unSources = 1; unSources < 9; ++unSources) {
            if (m_tSourceOperation[unSources] > m_unClock) {
                m_vArenaState[unSources] = 1;
                m_pcArena->SetBoxColor(2, unSources, CColor::GREEN);
            } else {
                m_vArenaState[unSources] = 0;
                m_pcArena->SetBoxColor(2, unSources, CColor::BLACK);
            }
        }
    } else {
        for (UInt32 unSources = 1; unSources < 9; ++unSources) {
            if (m_tSourceOperation[unSources] == m_unClock) {
                m_vArenaState[unSources] = 0;
                m_pcArena->SetBoxColor(2, unSources, CColor::BLACK);
            }
        }
    }

    if (m_bRealExperiments) {
        if (m_vArenaState != m_vArenaStatePast) {
            std::string sTmpCommand = m_sBaseCommand + "C";
            for (UInt32 i = 0; i < m_vArenaState.size(); i++)
                sTmpCommand += std::to_string(m_vArenaState[i]);
            sTmpCommand += m_sSufixCommand;
            system(sTmpCommand.c_str());
            m_vArenaStatePast = m_vArenaState;
        }
    }
}

/****************************************/
/****************************************/

void SeqLoopFunction::ScoreControl() {

    if (m_unEvalTask == 0) {
        if (m_bBlueFirst) {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
            else                          m_fObjectiveFunctionRed  += GetScore(m_unRedTask);
        } else {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionRed  += GetScore(m_unRedTask);
            else                          m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
        }
    }
    else if (m_unEvalTask == 1) {
        if (m_bBlueFirst) {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
        } else {
            if (m_unClock > m_unTrnTime)  m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
        }
    }
    else if (m_unEvalTask == 2) {
        if (!m_bBlueFirst) {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionRed += GetScore(m_unRedTask);
        } else {
            if (m_unClock > m_unTrnTime)  m_fObjectiveFunctionRed += GetScore(m_unRedTask);
        }
    }
    else if (m_unEvalTask == 3) {
        if (m_bBlueFirst) {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
            else                          m_fObjectiveFunctionRed  += GetScore(m_unRedTask);
        } else {
            if (m_unClock <= m_unTrnTime) m_fObjectiveFunctionRed  += GetScore(m_unRedTask);
            else                          m_fObjectiveFunctionBlue += GetScore(m_unBlueTask);
        }
    }
    else {
        m_fObjectiveFunction      = 888888;
        m_fObjectiveFunctionBlue  = 888888;
        m_fObjectiveFunctionRed   = 888888;
    }
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetScore(UInt32 unTask) {
    switch (unTask) {
        case 0: return -GetManufactureScore();
        case 1: return -GetTransportScore();
        case 2: return -GetCleaningScore();
        case 3: return -GetPickUpScore();
        case 4: return -GetDispersionScore();
        case 5: return -GetRechargeScore();
        default: return 999999;
    }
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetNormalizedScore(Real fScore, UInt32 unTask) {
    switch (unTask) {
        case 0: return fScore;
        case 1: return fScore;
        case 2: return fScore;
        case 3: return fScore;
        case 4: return fScore;
        case 5: return fScore;
        default: return 999999;
    }
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetRechargeScore() {
    UpdateRobotPositions();
    Real unScore = 0;
    TRobotStateMap::iterator it;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it)
        if (IsRobotInDock(it->second.cPosition)) unScore += 1;
    return unScore;
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetTransportScore() {
    UpdateRobotPositions();
    bool bInNest;
    UInt32 unInSource = 0;
    Real unScore = 0;
    TRobotStateMap::iterator it;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        if (it->second.bItem == true) {
            bInNest = IsRobotInNest(it->second.cPosition);
            if (bInNest) { unScore += 1; it->second.bItem = false; }
        } else {
            unInSource = IsRobotInSourceID(it->second.cPosition);
            if (unInSource != 0) it->second.bItem = true;
        }
    }
    return unScore;
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetManufactureScore() {
    UpdateRobotPositions();
    UInt32 unInSource = 0;
    Real unScore = 0;
    TRobotStateMap::iterator it;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        unInSource = IsRobotInSourceID(it->second.cPosition);
        if (unInSource != 0) {
            if (m_tSourceOperation[unInSource] > m_unClock)
                m_tSourceItems[unInSource] += 1;
            if (m_tSourceItems[unInSource] == 100) {
                m_tSourceItems[unInSource] = 0;
                unScore += 1;
            }
        }
    }
    return unScore;
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetPickUpScore() {
    UpdateRobotPositions();
    bool bInNest;
    Real unScore = 0;
    TRobotStateMap::iterator it;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        if (it->second.bMaterial == true) {
            bInNest = IsRobotInNest(it->second.cPosition);
            if (!bInNest) { unScore += 1; it->second.bMaterial = false; }
        } else {
            bInNest = IsRobotInNest(it->second.cPosition);
            if (bInNest) it->second.bMaterial = true;
        }
    }
    return unScore;
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetDispersionScore() {
    UpdateRobotPositions();
    Real unScore = 0;
    Real fMinDistance = 1.96;
    Real fDistance = 0;
    TRobotStateMap::iterator it, jt;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it)
        for (jt = m_tRobotStates.begin(); jt != it; ++jt) {
            fDistance = (it->second.cPosition - jt->second.cPosition).Length();
            if (fDistance < fMinDistance) fMinDistance = fDistance;
        }
    unScore = fMinDistance;
    return unScore;
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetCleaningScore() {
    UpdateRobotPositions();
    UInt32 unInSource = 0;
    Real unScore = 0;
    TRobotStateMap::iterator it;

    for (UInt32 unSources = 1; unSources < 9; ++unSources)
        m_tSourceRestoring[unSources] = 0;

    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        unInSource = IsRobotInSourceID(it->second.cPosition);
        if (unInSource != 0)
            if (m_tSourceOperation[unInSource] <= m_unClock)
                m_tSourceRestoring[unInSource] += 1;
    }

    for (UInt32 unSources = 1; unSources < 9; ++unSources) {
        if (m_tSourceOperation[unSources] <= m_unClock) {
            if (m_tSourceRestoring[unSources] >= 2)
                m_tSourceReparation[unSources] += 1;
            else
                m_tSourceReparation[unSources] = 0;
            if (m_tSourceReparation[unSources] == 50) {
                m_tSourceReparation[unSources] = 0;
                unScore += 1;
            }
        }
    }
    return unScore;
}

/****************************************/
/****************************************/

argos::CColor SeqLoopFunction::GetFloorColor(const argos::CVector2& c_position_on_plane) {

    if (c_position_on_plane.GetX() >= -0.125 && c_position_on_plane.GetX() <= 0.125) {
        if (c_position_on_plane.GetY() >= 0.65033 || c_position_on_plane.GetY() <= -0.65033)
            return CColor::BLACK;
        else if (c_position_on_plane.GetY() >= -0.125 && c_position_on_plane.GetY() <= 0.125)
            return CColor::WHITE;
    }
    else if (c_position_on_plane.GetY() >= -0.125 && c_position_on_plane.GetY() <= 0.125) {
        if (c_position_on_plane.GetX() >= 0.65033 || c_position_on_plane.GetX() <= -0.65033)
            return CColor::BLACK;
    }
    else if (c_position_on_plane.GetY() <=  c_position_on_plane.GetX() + 0.1767766953 &&
             c_position_on_plane.GetY() >=  c_position_on_plane.GetX() - 0.1767766953) {
        if (c_position_on_plane.GetY() >= -c_position_on_plane.GetX() + 0.9267766094 ||
            c_position_on_plane.GetY() <= -c_position_on_plane.GetX() - 0.9267766094)
            return CColor::BLACK;
    }
    else if (c_position_on_plane.GetY() <= -c_position_on_plane.GetX() + 0.1767766953 &&
             c_position_on_plane.GetY() >= -c_position_on_plane.GetX() - 0.1767766953) {
        if (c_position_on_plane.GetY() >=  c_position_on_plane.GetX() + 0.9267766094 ||
            c_position_on_plane.GetY() <=  c_position_on_plane.GetX() - 0.9267766094)
            return CColor::BLACK;
    }

    return CColor::GRAY50;
}

/****************************************/
/****************************************/

bool SeqLoopFunction::IsRobotInNest(CVector2 tRobotPosition) {
    return (tRobotPosition.GetX() >= -0.16 && tRobotPosition.GetX() <= 0.16 &&
            tRobotPosition.GetY() >= -0.16 && tRobotPosition.GetY() <= 0.16);
}

/****************************************/
/****************************************/

bool SeqLoopFunction::IsRobotInDock(CVector2 tRobotPosition) {
    if (tRobotPosition.Length() >= 0.65033) {
        if (tRobotPosition.GetX() >= -0.16 && tRobotPosition.GetX() <= 0.16) {
            if (tRobotPosition.GetY() >= 0.61533 || tRobotPosition.GetY() <= -0.61533)
                return false;
        }
        else if (tRobotPosition.GetY() >= -0.16 && tRobotPosition.GetY() <= 0.16) {
            if (tRobotPosition.GetX() >= 0.61533 || tRobotPosition.GetX() <= -0.61533)
                return false;
        }
        else if (tRobotPosition.GetY() <=  tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >=  tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >= -tRobotPosition.GetX() + 0.87727913472 ||
                tRobotPosition.GetY() <= -tRobotPosition.GetX() - 0.87727913472)
                return false;
        }
        else if (tRobotPosition.GetY() <= -tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >= -tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >=  tRobotPosition.GetX() + 0.87727913472 ||
                tRobotPosition.GetY() <=  tRobotPosition.GetX() - 0.87727913472)
                return false;
        }
        return true;
    }
    return false;
}

/****************************************/
/****************************************/

bool SeqLoopFunction::IsRobotInSource(CVector2 tRobotPosition) {
    if (tRobotPosition.Length() >= 0.6) {
        if (tRobotPosition.GetX() >= -0.16 && tRobotPosition.GetX() <= 0.16) {
            if (tRobotPosition.GetY() >= 0.61533 || tRobotPosition.GetY() <= -0.61533)
                return true;
        }
        else if (tRobotPosition.GetY() >= -0.16 && tRobotPosition.GetY() <= 0.16) {
            if (tRobotPosition.GetX() >= 0.61533 || tRobotPosition.GetX() <= -0.61533)
                return true;
        }
        else if (tRobotPosition.GetY() <=  tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >=  tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >= -tRobotPosition.GetX() + 0.87727913472 ||
                tRobotPosition.GetY() <= -tRobotPosition.GetX() - 0.87727913472)
                return true;
        }
        else if (tRobotPosition.GetY() <= -tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >= -tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >=  tRobotPosition.GetX() + 0.87727913472 ||
                tRobotPosition.GetY() <=  tRobotPosition.GetX() - 0.87727913472)
                return true;
        }
    }
    return false;
}

/****************************************/
/****************************************/

UInt32 SeqLoopFunction::IsRobotInSourceID(CVector2 tRobotPosition) {
    UInt32 unSourceId = 0;
    if (tRobotPosition.Length() >= 0.6) {
        if (tRobotPosition.GetX() >= -0.16 && tRobotPosition.GetX() <= 0.16) {
            if (tRobotPosition.GetY() >= 0.61533)       unSourceId = 2;
            else if (tRobotPosition.GetY() <= -0.61533) unSourceId = 6;
        }
        else if (tRobotPosition.GetY() >= -0.16 && tRobotPosition.GetY() <= 0.16) {
            if (tRobotPosition.GetX() >= 0.61533)       unSourceId = 8;
            else if (tRobotPosition.GetX() <= -0.61533) unSourceId = 4;
        }
        else if (tRobotPosition.GetY() <=  tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >=  tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >= -tRobotPosition.GetX() + 0.87727913472)       unSourceId = 1;
            else if (tRobotPosition.GetY() <= -tRobotPosition.GetX() - 0.87727913472)  unSourceId = 5;
        }
        else if (tRobotPosition.GetY() <= -tRobotPosition.GetX() + 0.22450640303 &&
                 tRobotPosition.GetY() >= -tRobotPosition.GetX() - 0.22450640303) {
            if (tRobotPosition.GetY() >=  tRobotPosition.GetX() + 0.87727913472)       unSourceId = 3;
            else if (tRobotPosition.GetY() <=  tRobotPosition.GetX() - 0.87727913472)  unSourceId = 7;
        }
    }
    return unSourceId;
}

/****************************************/
/****************************************/

void SeqLoopFunction::UpdateRobotPositions() {
    CSpace::TMapPerType& tEpuckMap = GetSpace().GetEntitiesByType("epuck");
    CVector2 cEpuckPosition(0, 0);
    for (CSpace::TMapPerType::iterator it = tEpuckMap.begin(); it != tEpuckMap.end(); ++it) {
        CEPuckEntity* pcEpuck = any_cast<CEPuckEntity*>(it->second);
        cEpuckPosition.Set(pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
                           pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetY());
        m_tRobotStates[pcEpuck].cLastPosition = m_tRobotStates[pcEpuck].cPosition;
        m_tRobotStates[pcEpuck].cPosition = cEpuckPosition;
    }
}

/****************************************/
/****************************************/

void SeqLoopFunction::UpdateRobotColors() {
    CSpace::TMapPerType& tEpuckMap = GetSpace().GetEntitiesByType("epuck");
    CColor cEpuckColor;
    for (CSpace::TMapPerType::iterator it = tEpuckMap.begin(); it != tEpuckMap.end(); ++it) {
        CEPuckEntity* pcEpuck = any_cast<CEPuckEntity*>(it->second);
        cEpuckColor = pcEpuck->GetLEDEquippedEntity().GetLED(10).GetColor();
        m_tRobotStates[pcEpuck].cColor = cEpuckColor;
    }
}

/****************************************/
/****************************************/

void SeqLoopFunction::InitRobotStates() {
    CSpace::TMapPerType& tEpuckMap = GetSpace().GetEntitiesByType("epuck");
    CVector2 cEpuckPosition(0, 0);
    for (CSpace::TMapPerType::iterator it = tEpuckMap.begin(); it != tEpuckMap.end(); ++it) {
        CEPuckEntity* pcEpuck = any_cast<CEPuckEntity*>(it->second);
        cEpuckPosition.Set(pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
                           pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetY());
        m_tRobotStates[pcEpuck].cLastPosition = cEpuckPosition;
        m_tRobotStates[pcEpuck].cPosition     = cEpuckPosition;
        m_tRobotStates[pcEpuck].cColor        = CColor::BLACK;
        m_tRobotStates[pcEpuck].bItem         = false;
        m_tRobotStates[pcEpuck].bMaterial     = false;
        m_tRobotStates[pcEpuck].bMoving       = false;
    }
}

/****************************************/
/****************************************/

void SeqLoopFunction::InitSources() {
    UInt32 unExpTime = 2 * m_unTrnTime;
    for (UInt32 unSources = 1; unSources < 9; ++unSources) {
        m_tSourceItems[unSources]      = 0;
        m_tSourceOperation[unSources]  = GetRandomTime(1, unExpTime);
        m_tSourceRestoring[unSources]  = 0;
        m_tSourceReparation[unSources] = 0;
    }
    m_tSourceOperation[2] = unExpTime;
}

/****************************************/
/****************************************/

bool SeqLoopFunction::SelectColorOrder(UInt32 un_ColorOrderParam) {
    Real fRandomIndex;
    bool bBlueFirst = true;
    if (un_ColorOrderParam == 0)
        fRandomIndex = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
    else
        fRandomIndex = (un_ColorOrderParam * 0.5) - (0.5 / 2);
    if (fRandomIndex > 0.5) bBlueFirst = false;
    return bBlueFirst;
}

/****************************************/
/****************************************/

void SeqLoopFunction::AsignArenaColors(UInt32 un_NumberColorsParam) {
    if (un_NumberColorsParam == 0) {
        m_cTaskAsignedColorBlue = CColor::BLACK;
        m_cTaskEvalColorBlue    = CColor::BLACK;
        m_cTaskAsignedColorRed  = CColor::BLACK;
        m_cTaskEvalColorRed     = CColor::BLACK;
    } else if (un_NumberColorsParam == 1) {
        m_cTaskAsignedColorBlue = CColor::BLACK;
        m_cTaskEvalColorBlue    = CColor::BLACK;
        m_cTaskAsignedColorRed  = CColor::RED;
        m_cTaskEvalColorRed     = CColor::MAGENTA;
    } else {
        m_cTaskAsignedColorBlue = CColor::BLUE;
        m_cTaskEvalColorBlue    = CColor::CYAN;
        m_cTaskAsignedColorRed  = CColor::RED;
        m_cTaskEvalColorRed     = CColor::MAGENTA;
    }
}

/****************************************/
/****************************************/

void SeqLoopFunction::GetArenaPoints(UInt32 unNumberPoints) {
    CVector2 cRandomPoint;
    for (UInt32 unPoint = 0; unPoint < unNumberPoints; unPoint++) {
        cRandomPoint = GetRandomArenaPoint();
        m_tArenaPoints.push_back(cRandomPoint);
    }
}

/****************************************/
/****************************************/

CVector2 SeqLoopFunction::GetRandomArenaPoint() {
    Real a = m_pcRng->Uniform(CRange<Real>(0.0f, 1000.0f));
    Real b = m_pcRng->Uniform(CRange<Real>(0.0f, 1000.0f));
    CVector2 cArenaPoint;
    cArenaPoint.FromPolarCoordinates((a / 1000) * 0.98, (b / 1000) * CRadians::TWO_PI);
    return cArenaPoint;
}

/****************************************/
/****************************************/

CVector3 SeqLoopFunction::GetRandomPosition() {
    Real temp;
    Real a = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
    Real b = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
    Real c = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
    Real d = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
    if (b < a) { temp = a; a = b; b = temp; }
    // LLM_MISSION_DISTRIBUTION_RATIO : le rayon suit la taille de l'arene au lieu
    // d'etre fige a 0.4, sinon les petites arenes font demarrer les
    // robots colles au mur et le moteur 2D en ejecte un.
    m_fDistributionRadius = m_fDistributionRatio * GetArenaRadious();
    Real fPosX = (c * m_fDistributionRadius / 2) + m_fDistributionRadius * cos(2 * CRadians::PI.GetValue() * (a / b));
    Real fPosY = (d * m_fDistributionRadius / 2) + m_fDistributionRadius * sin(2 * CRadians::PI.GetValue() * (a / b));
    return CVector3(fPosX, fPosY, 0);
}

/****************************************/
/****************************************/

UInt32 SeqLoopFunction::GetRandomTime(UInt32 unMin, UInt32 unMax) {
    return m_pcRng->Uniform(CRange<UInt32>(unMin, unMax));
}

/****************************************/
/****************************************/

void SeqLoopFunction::PositionArena() {
    CArenaEntity* pcArena;
    pcArena = new CArenaEntity("arena",
                               CVector3(0, 0, 0),
                               CQuaternion().FromEulerAngles(CRadians::ZERO, CRadians::ZERO, CRadians::ZERO),
                               CVector3(0.01, m_fLenghtBoxes, 0.1),
                               "leds",
                               m_unNumberBoxes,
                               m_unNumberEdges,
                               0.125f,
                               1.0f);
    AddEntity(*pcArena);
    m_pcArena = pcArena;
}

/****************************************/
/****************************************/

void SeqLoopFunction::RemoveArena() {
    std::ostringstream id;
    id << "arena";
    RemoveEntity(id.str().c_str());
}

/****************************************/
/****************************************/

Real SeqLoopFunction::GetArenaRadious() {
    Real fRadious = (m_fLenghtBoxes * m_unNumberBoxes) / (2 * Tan(CRadians::PI / m_unNumberEdges));
    fRadious = fRadious - 0.65;
    return fRadious;
}

/****************************************/
/****************************************/

bool SeqLoopFunction::IsEven(UInt32 unNumber) {
    return ((unNumber % 2) == 0);
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(SeqLoopFunction, "seq_loop_function");

/**
  * @file <loop-functions/IcraLoopFunc.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @license MIT License
  */

#include "TuttiFrLoopFunc.h"

/****************************************/
/****************************************/

TuttiFrLoopFunction::TuttiFrLoopFunction() {
    m_unClock = 0;
    m_fObjectiveFunction = 0;
}

/****************************************/
/****************************************/

TuttiFrLoopFunction::TuttiFrLoopFunction(const TuttiFrLoopFunction& orig) {
}

/****************************************/
/****************************************/

TuttiFrLoopFunction::~TuttiFrLoopFunction() {}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::Destroy() {

    m_tRobotStates.clear();
    RemoveArena();
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::Init(TConfigurationNode& t_tree) {

    CoreLoopFunctions::Init(t_tree);
    TConfigurationNode cParametersNode;
    try {
      cParametersNode = GetNode(t_tree, "params");
      GetNodeAttributeOrDefault(cParametersNode, "build_arena", m_bBuildArena, (bool) false);
      GetNodeAttributeOrDefault(cParametersNode, "number_edges", m_unNumberEdges, (UInt32) 3);
      GetNodeAttributeOrDefault(cParametersNode, "number_boxes_per_edge", m_unNumberBoxes, (UInt32) 1);
      GetNodeAttributeOrDefault(cParametersNode, "lenght_boxes", m_fLenghtBoxes, (Real) 0.25);
      GetNodeAttributeOrDefault(cParametersNode, "maximization", m_bMaximization, (bool) false);
    } catch(std::exception e) {
    }

    if (m_bBuildArena == true)
    {
        m_fDistributionRadius = GetArenaRadious();
        PositionArena();
    }

    InitRobotStates();

}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::Reset() {
    CoreLoopFunctions::Reset();

    m_unClock = 0;
    m_fObjectiveFunction = 0;

    m_tRobotStates.clear();

    InitRobotStates();
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::PostStep() {

    m_unClock = GetSpace().GetSimulationClock();

    ScoreControl();
    ArenaControl();

}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::PostExperiment() {
    if (m_bMaximization == true){
        LOG << -m_fObjectiveFunction << std::endl;
    }
    else {
        LOG << m_fObjectiveFunction << std::endl;
    }
}

/****************************************/
/****************************************/

Real TuttiFrLoopFunction::GetObjectiveFunction() {
    if (m_bMaximization == true){
        return -m_fObjectiveFunction;
    }
    else {
        return m_fObjectiveFunction;
    }
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::ArenaControl() {

    if (m_unClock == 1) {
        m_pcArena->SetBoxColor(1,4,CColor::BLUE);
        m_pcArena->SetBoxColor(2,4,CColor::BLUE);
        m_pcArena->SetBoxColor(3,4,CColor::BLUE);
        m_pcArena->SetBoxColor(4,4,CColor::GREEN);
        m_pcArena->SetBoxColor(5,4,CColor::GREEN);
        m_pcArena->SetBoxColor(6,4,CColor::GREEN);
        m_pcArena->SetWallColor(2,CColor::RED);
    }


    return;
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::ScoreControl(){

    m_fObjectiveFunction += -GetForageScore();
}

/****************************************/
/****************************************/

Real TuttiFrLoopFunction::GetForageScore() {

    UpdateRobotPositions();

    bool bInNest;
    UInt32 unInSource;
    Real unScore = 0;
    TRobotStateMap::iterator it;

    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {

        if (it->second.unItem != 0){

            unInSource = IsRobotInSourceID(it->second.cPosition);
            if (unInSource == 1) {
                it->second.unItem = 1;
            }
            else if (unInSource == 2) {
                it->second.unItem = 2;
            }
            else {
                bInNest = IsRobotInNest(it->second.cPosition);
                if (bInNest) {
                    if (it->second.unItem == 1) {
                        unScore-=1;
                    }
                    else if (it->second.unItem == 2) {
                        unScore+=1;
                    }
                    it->second.unItem = 0;
                }
            }
        }
        else {
            unInSource = IsRobotInSourceID(it->second.cPosition);
            if (unInSource == 1) {
                it->second.unItem = 1;
            }
            else if (unInSource == 2) {
                it->second.unItem = 2;
            }
        }
    }

    return unScore;
}

/****************************************/
/****************************************/

argos::CColor TuttiFrLoopFunction::GetFloorColor(const argos::CVector2& c_position_on_plane) {

    if (c_position_on_plane.GetX() <= -0.60){
        return CColor::WHITE;
    }
    else if (c_position_on_plane.GetX() >= 0.60){
        return CColor::BLACK;
    }

    return CColor::GRAY50;
}

/****************************************/
/****************************************/

bool TuttiFrLoopFunction::IsRobotInNest (CVector2 tRobotPosition) {

    if (tRobotPosition.GetX() <= -0.565)
        return true;

    return false;
}

/****************************************/
/****************************************/

UInt32 TuttiFrLoopFunction::IsRobotInSourceID (CVector2 tRobotPosition){

    UInt32 unSourceId = 0;

    if (tRobotPosition.GetX() >= 0.565) {

        if (tRobotPosition.GetY() <= -0.006)
            unSourceId = 1;
        else if (tRobotPosition.GetY() >= 0.006)
            unSourceId = 2;
    }

    return unSourceId;
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::UpdateRobotPositions() {
    CSpace::TMapPerType& tEpuckMap = GetSpace().GetEntitiesByType("epuck");
    CVector2 cEpuckPosition(0,0);
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

void TuttiFrLoopFunction::InitRobotStates() {

    CSpace::TMapPerType& tEpuckMap = GetSpace().GetEntitiesByType("epuck");
    CVector2 cEpuckPosition(0,0);
    for (CSpace::TMapPerType::iterator it = tEpuckMap.begin(); it != tEpuckMap.end(); ++it) {
        CEPuckEntity* pcEpuck = any_cast<CEPuckEntity*>(it->second);
        cEpuckPosition.Set(pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
                           pcEpuck->GetEmbodiedEntity().GetOriginAnchor().Position.GetY());

        m_tRobotStates[pcEpuck].cLastPosition = cEpuckPosition;
        m_tRobotStates[pcEpuck].cPosition = cEpuckPosition;
        m_tRobotStates[pcEpuck].unItem = 0;
    }
}

/****************************************/
/****************************************/

CVector3 TuttiFrLoopFunction::GetRandomPosition() {
  Real temp;
  Real a = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
  Real b = m_pcRng->Uniform(CRange<Real>(0.0f, 1.0f));
  Real c = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
  Real d = m_pcRng->Uniform(CRange<Real>(-1.0f, 1.0f));
  // If b < a, swap them
  if (b < a) {
    temp = a;
    a = b;
    b = temp;
  }
  m_fDistributionRadius = 0.4;
  Real fPosX = (c * m_fDistributionRadius / 2) + m_fDistributionRadius * cos(2 * CRadians::PI.GetValue() * (a/b));
  Real fPosY = (d * m_fDistributionRadius / 2) + m_fDistributionRadius * sin(2 * CRadians::PI.GetValue() * (a/b));

  return CVector3(fPosX, fPosY, 0);
}

/****************************************/
/****************************************/

void TuttiFrLoopFunction::PositionArena() {
  CArenaEntity* pcArena;
  /*
    pcArena = new CArenaEntity("arena",
                               CVector3(0,0,0),
                               CQuaternion().FromEulerAngles(CRadians::ZERO,CRadians::ZERO,CRadians::ZERO), // TODO
                               CVector3(0.01,m_fLenghtBoxes,0.1),
                               "leds",
                               m_unNumberBoxes,
                               m_unNumberEdges,
                               0.017f,
                               1.0f);   */ // arena with 12 leds per block

  pcArena = new CArenaEntity("arena",
                             CVector3(0,0,0),
                             CQuaternion().FromEulerAngles(CRadians::ZERO,CRadians::ZERO,CRadians::ZERO), // TODO
                             CVector3(0.01,m_fLenghtBoxes,0.1),
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

void TuttiFrLoopFunction::RemoveArena() {
    std::ostringstream id;
    id << "arena";
    RemoveEntity(id.str().c_str());
}

/****************************************/
/****************************************/

Real TuttiFrLoopFunction::GetArenaRadious() {

    Real fRadious;
    fRadious =  (m_fLenghtBoxes*m_unNumberBoxes) / (2 * Tan(CRadians::PI / m_unNumberEdges));
    //fRadious = fRadious - 0.10; // Avoids to place robots close the walls.
    fRadious = fRadious - 0.65; // Reduced cluster at the begining

    return fRadious;
}

/****************************************/
/****************************************/

bool TuttiFrLoopFunction::IsEven(UInt32 unNumber) {
    bool even;
    if((unNumber%2)==0)
       even = true;
    else
       even = false;

    return even;
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(TuttiFrLoopFunction, "tutti_fr_loop_function");

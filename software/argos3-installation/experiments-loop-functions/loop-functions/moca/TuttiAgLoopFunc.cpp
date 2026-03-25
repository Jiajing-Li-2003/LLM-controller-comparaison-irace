/**
  * @file <loop-functions/IcraLoopFunc.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @license MIT License
  */

#include "TuttiAgLoopFunc.h"

/****************************************/
/****************************************/

TuttiAgLoopFunction::TuttiAgLoopFunction() {
    m_unClock = 0;
    m_fObjectiveFunction = 0;
}

/****************************************/
/****************************************/

TuttiAgLoopFunction::TuttiAgLoopFunction(const TuttiAgLoopFunction& orig) {
}

/****************************************/
/****************************************/

TuttiAgLoopFunction::~TuttiAgLoopFunction() {}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::Destroy() {

    m_tRobotStates.clear();
    RemoveArena();
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::Init(TConfigurationNode& t_tree) {

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
    m_pcArena->SetArenaColor(CColor::BLACK);

}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::Reset() {
    CoreLoopFunctions::Reset();

    m_unClock = 0;
    m_fObjectiveFunction = 0;

    m_tRobotStates.clear();

    InitRobotStates();
    m_pcArena->SetArenaColor(CColor::BLACK);
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::PostStep() {

    m_unClock = GetSpace().GetSimulationClock();

    ScoreControl();
    ArenaControl();

}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::PostExperiment() {
    if (m_bMaximization == true){
        LOG << -m_fObjectiveFunction << std::endl;
    }
    else {
        LOG << m_fObjectiveFunction << std::endl;
    }
}

/****************************************/
/****************************************/

Real TuttiAgLoopFunction::GetObjectiveFunction() {
    if (m_bMaximization == true){
        return -m_fObjectiveFunction;
    }
    else {
        return m_fObjectiveFunction;
    }
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::ArenaControl() {

    if (m_unClock == 1) {
        m_pcArena->SetWallColor(1,CColor::BLUE);
        m_pcArena->SetWallColor(2,CColor::BLUE);
        m_pcArena->SetWallColor(4,CColor::GREEN);
        m_pcArena->SetWallColor(5,CColor::GREEN);
    }

    return;
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::ScoreControl(){

    m_fObjectiveFunction += GetAggregationScore();
}

/****************************************/
/****************************************/

Real TuttiAgLoopFunction::GetAggregationScore() {

    UpdateRobotPositions();

    bool bInAgg;
    Real unScore = 0;
    TRobotStateMap::iterator it;
    for (it = m_tRobotStates.begin(); it != m_tRobotStates.end(); ++it) {
        bInAgg = IsRobotInAgg(it->second.cPosition);
        if (!bInAgg)
            unScore+=1;
    }

    return unScore;
}

/****************************************/
/****************************************/

argos::CColor TuttiAgLoopFunction::GetFloorColor(const argos::CVector2& c_position_on_plane) {

    if (c_position_on_plane.GetY() <= -0.50 || c_position_on_plane.GetY() >= 0.50){
        return CColor::BLACK;
    }

    return CColor::GRAY50;
}


/****************************************/
/****************************************/

bool TuttiAgLoopFunction::IsRobotInAgg (CVector2 tRobotPosition) {

    if (tRobotPosition.GetY() >= 0.465){
        return true;
    }

    return false;
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::UpdateRobotPositions() {
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

void TuttiAgLoopFunction::InitRobotStates() {

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

CVector3 TuttiAgLoopFunction::GetRandomPosition() {
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
  m_fDistributionRadius = 0.25;
  Real fPosX = (c * m_fDistributionRadius) + m_fDistributionRadius * cos(2 * CRadians::PI.GetValue() * (a/b));
  Real fPosY = (d * m_fDistributionRadius) + m_fDistributionRadius * sin(2 * CRadians::PI.GetValue() * (a/b));

  return CVector3(fPosX, fPosY, 0);
}

/****************************************/
/****************************************/

void TuttiAgLoopFunction::PositionArena() {
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

void TuttiAgLoopFunction::RemoveArena() {
    std::ostringstream id;
    id << "arena";
    RemoveEntity(id.str().c_str());
}

/****************************************/
/****************************************/

Real TuttiAgLoopFunction::GetArenaRadious() {

    Real fRadious;
    fRadious =  (m_fLenghtBoxes*m_unNumberBoxes) / (2 * Tan(CRadians::PI / m_unNumberEdges));
    //fRadious = fRadious - 0.10; // Avoids to place robots close the walls.
    fRadious = fRadious - 0.65; // Reduced cluster at the begining

    return fRadious;
}

/****************************************/
/****************************************/

bool TuttiAgLoopFunction::IsEven(UInt32 unNumber) {
    bool even;
    if((unNumber%2)==0)
       even = true;
    else
       even = false;

    return even;
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(TuttiAgLoopFunction, "tutti_ag_loop_function");

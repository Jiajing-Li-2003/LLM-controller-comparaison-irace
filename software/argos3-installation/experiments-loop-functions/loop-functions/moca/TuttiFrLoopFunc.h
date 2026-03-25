/**
  * @file <loop-functions/example/PwLoopFunc.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef TUTTI_FR_LOOP_FUNC
#define TUTTI_FR_LOOP_FUNC

#include <argos3/core/simulator/space/space.h>
#include <argos3/plugins/robots/e-puck/simulator/epuck_entity.h>
#include <argos3/plugins/simulator/entities/box_entity.h>
#include <argos3/plugins/robots/arena/simulator/arena_entity.h>
#include "../../src/CoreLoopFunctions.h"

using namespace argos;

class TuttiFrLoopFunction: public CoreLoopFunctions {
  public:
    TuttiFrLoopFunction();
    TuttiFrLoopFunction(const TuttiFrLoopFunction& orig);
    virtual ~TuttiFrLoopFunction();

    virtual void Destroy();

    virtual argos::CColor GetFloorColor(const argos::CVector2& c_position_on_plane);
    virtual void PostExperiment();
    virtual void PostStep();
    virtual void Reset();
    virtual void Init(TConfigurationNode& t_tree);

    Real GetObjectiveFunction();

    CVector3 GetRandomPosition();
    UInt32 GetRandomTime(UInt32 unMin, UInt32 unMax);

    void ArenaControl();

    void InitRobotStates();
    void UpdateRobotPositions();

    bool IsRobotInNest (CVector2 tRobotPosition);
    UInt32 IsRobotInSourceID (CVector2 tRobotPosition);

    void ScoreControl();
    Real GetForageScore();

  private:

    /*
     * Method used to create and distribute the Arena.
     */
    void PositionArena();

    /*
     * Method used to remove the arena from the arena.
     */
    void RemoveArena();

    /*
     * Method used to deternmine wheter a number is even.
     */
    bool IsEven(UInt32 unNumber);

    /*
     * Return the radious of the arena.
     */
    Real GetArenaRadious();

    /*
     * Build the arena with the arena_entity plugin.
     */
    bool m_bBuildArena;

    /*
     * The number of edges in the arena used in the experiment.
     */
    UInt32 m_unNumberEdges;

    /*
     * The number of boxes in each edge of the arena used in the experiment.
     */
    UInt32 m_unNumberBoxes;

    /*
     * The lenght of the boxes used in the experiment.
     */
    Real m_fLenghtBoxes;

    /*
     * The arena used in the experiment.
     */
    CArenaEntity* m_pcArena;

    /*
     * Transition time in sequence experiments
     */
    UInt32 m_unTrnTime;

    /*
     * Allows for score invertion in maximization algorithms.
     */
    bool m_bMaximization;

    UInt32 m_unClock;
    Real m_fObjectiveFunction;

    struct RobotStateStruct {
        CVector2 cLastPosition;
        CVector2 cPosition;
        UInt32 unItem;
    };

    typedef std::map<CEPuckEntity*, RobotStateStruct> TRobotStateMap;

    TRobotStateMap m_tRobotStates;

};

#endif

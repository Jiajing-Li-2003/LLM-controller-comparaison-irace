/*
 * @file <src/core/AutoMoDeFiniteStateMachine.h>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 *
 * @brief	This class represents the stochastic Finite State
 * 				Machine (FSM) that controls the robot. It contains all the modules
 *       	(behaviours and conditions) and is responsible for the transitions
 * 				between them.
 */

#ifndef AUTOMODE_FINITE_STATE_MACHINE_H
#define AUTOMODE_FINITE_STATE_MACHINE_H

#include <argos3/demiurge/epuck-dao/EpuckDAO.h>

#include "AutoMoDeFsmHistory.h"
#include "../modules/AutoMoDeBehaviour.h"
#include "../modules/AutoMoDeBehaviourAttraction.h"
#include "../modules/AutoMoDeBehaviourAntiPhototaxis.h"
#include "../modules/AutoMoDeBehaviourExploration.h"
#include "../modules/AutoMoDeBehaviourPhototaxis.h"
#include "../modules/AutoMoDeBehaviourRepulsion.h"
#include "../modules/AutoMoDeBehaviourStop.h"
#include "../modules/AutoMoDeBehaviourGoToColor.h"
#include "../modules/AutoMoDeBehaviourGoAwayColor.h"
#include "../modules/AutoMoDeCondition.h"
#include "../modules/AutoMoDeConditionBlackFloor.h"
#include "../modules/AutoMoDeConditionGrayFloor.h"
#include "../modules/AutoMoDeConditionWhiteFloor.h"
#include "../modules/AutoMoDeConditionFixedProbability.h"
#include "../modules/AutoMoDeConditionNeighborsCount.h"
#include "../modules/AutoMoDeConditionInvertedNeighborsCount.h"
#include "../modules/AutoMoDeConditionProbColor.h"

#include <string>
#include <ctime>
#include <algorithm>

namespace argos {
	class AutoMoDeFiniteStateMachine {

		public:

			/*
			 * Class constructor.
			 */
			AutoMoDeFiniteStateMachine();

			/*
			 * Class destructor.
			 */
			virtual ~AutoMoDeFiniteStateMachine();

			/*
			 * Copy constructor.
			 * Intensively used in the main to duplicate the FSM to all robots.
			 */
			AutoMoDeFiniteStateMachine(const AutoMoDeFiniteStateMachine* pc_fsm);

			/*
			 * Add a condition to the FSM.
			 */
			void AddCondition(AutoMoDeCondition* pc_new_condition);

			/*
			 * Add a condition to the FSM.
			 */
			void AddBehaviour(AutoMoDeBehaviour* pc_new_behaviour);

			/*
			 * Core of the Finite State Machine. Responsible for the transitions
			 * between the different states (behaviours). Works as follows:
			 *   1. Execute current behaviour
			 *   2. Check for conditions (if current behaviour is not in blocking state)
			 *   3. Update current behaviour if one condition passed
			 *   4. Add entry to history if enabled
			 *   5. Update internal variables
			 */
			void ControlStep();

			/*
			 * Initialize the Finite State Machine.
			 */
			void Init();

			/*
			 * Reset the Finite State Machine.
			 */
			void Reset();

			/**
			 * Creates an URL containing a DOT description of the finite state machine.
			 */
			const std::string GetReadableFormat();

			/**
			 * Creates a AutoMoDeFsmHistory.
			 */
			void MaintainHistory();

			/*
			 * Returns the index of the behaviour corresponding to the current state of the FSM.
			 */
			const UInt32& GetCurrentBehaviourIndex() const;

			/*
			 * Returns the current time step.
			 */
			const UInt32& GetTimeStep() const;

			/*
			 * Returns all the behaviours contained in the FSM.
			 */
			std::vector<AutoMoDeBehaviour*> GetBehaviours() const;

			/*
			 * Returns all the conditions contained in the FSM.
			 */
			std::vector<AutoMoDeCondition*> GetConditions() const;

			/*
			 * Set the pointer to the class representing the state of the robot.
			 * @see EpuckDAO.
			 */
			void SetRobotDAO(EpuckDAO* m_pcRobotDAO);

			/*
			 * Setter for the finite state machine history folder;
			 */
			void SetHistoryFolder(const std::string& s_hist_folder);

		private:
			/*
			 * List of possible behaviours of the FSM.
			 */
			std::vector<AutoMoDeBehaviour*> m_vecBehaviours;

			/*
			 * List of possible conditions of the FSM.
			 */
			std::vector<AutoMoDeCondition*> m_vecConditions;

			/*
			 * Pointer to the behaviour associated with the active state of the FSM.
			 */
			AutoMoDeBehaviour* m_pcCurrentBehaviour;

			/*
			 * List of the conditions going out of the active state.
			 * These conditions will be checked and determine the next state of the FSM.
			 */
			std::vector<AutoMoDeCondition*> m_vecCurrentConditions;

			/*
			 * Pointer to the object keeping track of the successive
			 * states of the FSM.
			 */
			AutoMoDeFsmHistory* m_pcHistory;

			/*
			 * The index of the behaviour corresponding to the current
			 * active state of the FSM.
			 */
			UInt32 m_unCurrentBehaviourIndex;

			/*
			 * Flag indicating if an history of the visited states
			 * of the FSM is maintained.
			 */
			bool m_bMaintainHistory;

			/*
			 * The path to where the history shall be stored.
			 */
			std::string m_strHistoryFolder;

			/*
			 * Flag indicating if the FSM is changing state.
			 */
			bool m_bEnteringNewState;

			/*
			 * The current time step.
			 */
			UInt32 m_unTimeStep;

			/*
			 * Map containing pointers to tested outgoing conditions and
			 * the result of the test.
			 * @see AutoMoDeFsmHistory.
			 */
			std::map<AutoMoDeCondition*, bool> m_mapConditionsChecked;

			/*
			 * Pointer to the object representing the state of the robot.
			 * @see EpuckDAO.
			 */
			EpuckDAO* m_pcRobotDAO;

			/*
			 * Returns a container filled conditions starting from the
			 * current behaviour and finishing to possible future behaviours.
			 */
			std::vector<AutoMoDeCondition*> GetOutgoingConditions();

			/*
			 * Returns the DOT description of the initial state.
			 * @see GetReadableFormat()
			 */
			const std::string FillWithInitialState();

			/*
			 * Returns the DOT description of all the non initial states.
			 * @see GetReadableFormat()
			 */
			const std::string FillWithNonInitialStates();

			/*
			 * Returns the DOT description of the conditions.
			 * @see GetReadableFormat()
			 */
			const std::string FillWithConditions();

			/*
			 * Returns a pointer to the AutoMoDeFsmHistory object.
			 */
			AutoMoDeFsmHistory* GetHistory() const;

			/**
			 * Pass the pointer to the RobotDAO object to all modules part of the FSM.
			 */
			void ShareRobotDAO();

			/*
			 * Returns the flag indicating wether an history is maintained or not.
			 */
			const bool GetMaintainHistoryFlag() const;

			/*
			 * Returns the flag indicating wether the FSM is changing state.
			 */
			const bool GetEnteringNewStateFlag() const;
	};
}

#endif

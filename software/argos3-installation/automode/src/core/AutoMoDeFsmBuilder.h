/*
 * @file <src/core/AutoMoDeFsmBuilder.h>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 *
 * @brief This class is used to parse the configuration of
 * 				the stochastic finite state machine and instanciate the
 * 				different modules (behaviours and conditions). These modules
 * 				are added to the AutoMoDeFiniStateMachine created.
 */

#ifndef AUTOMODE_FSM_BUILDER_H
#define AUTOMODE_FSM_BUILDER_H

#include "AutoMoDeFiniteStateMachine.h"

#include <argos3/core/utility/logging/argos_log.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <stdlib.h>

namespace argos {
	class AutoMoDeFsmBuilder {
		public:
			/*
			 * Class constructor.
			 */
			AutoMoDeFsmBuilder();

			/**
			 * Creates an AutoMoDeFiniteStateMachine based on a configuration as a vector of strings.
			 * This method should be called when the FSM is created from the AutoMoDeMain.cpp.
			 */
			AutoMoDeFiniteStateMachine* BuildFiniteStateMachine(std::vector<std::string>& vec_fsm_config);

			/**
			 * Creates an AutoMoDeFiniteStateMachine based on a configuration as a string.
			 * This method should be called when the FSM is created from the AutoMoDeController.cpp.
			 */
			AutoMoDeFiniteStateMachine* BuildFiniteStateMachine(const std::string& str_fsm_config);

			/*
			 * Class destructor.
			 */
			virtual ~AutoMoDeFsmBuilder();

		private:
			/**
			 * Creates a AutoMoDeBehaviour from a state configuration and add it to the
			 * AutoMoDeFiniStateMachine in construction.
			 * Strips the different transitions and calls HandleTransition for their creation.
			 */
			void HandleState(AutoMoDeFiniteStateMachine* c_fsm, std::vector<std::string>& vec_fsm_state_config);

			/**
			 * Creates a AutoMoDeCondition from a transition configuration and add it to the
			 * AutoMoDeFiniStateMachine in construction.
			 */
			void HandleTransition(std::vector<std::string>& vec_fsm_transition_config,
									const UInt32& un_initial_state_index, const UInt32& un_condition_index);

			/**
			 * Creates a list containing the indexes of the behaviours reachable from a given state.
			 * Added for compatibility with irace interdependent parameters. 
			 */
			const std::vector<UInt32> GetPossibleDestinationBehaviour(const UInt32& un_initial_state_index);

			UInt32 m_unNumberStates;

			AutoMoDeFiniteStateMachine* cFiniteStateMachine;

	};
}

#endif

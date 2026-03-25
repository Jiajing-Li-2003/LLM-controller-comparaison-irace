/*
 * @file <src/core/AutoMoDeFsmHistory.h>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 *
 * @brief This class allows for keeping track of the
 * 				evolution of the visited states of the finite state machine.
 * 				The succession of behaviours controlling the robot are
 * 				thus registered and stored into a file.
 */

#ifndef AUTOMODE_FSM_HISTORY_H
#define AUTOMODE_FSM_HISTORY_H

#include "../modules/AutoMoDeBehaviour.h"
#include "../modules/AutoMoDeCondition.h"

#include <argos3/core/utility/logging/argos_log.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

namespace argos {
	class AutoMoDeFsmHistory {
		public:
			/*
			 * Class constructor. Takes the path to file where the history will be saved.
			 */
			AutoMoDeFsmHistory(const std::string& str_path);

			/*
			 * Copy constructor.
			 */
			AutoMoDeFsmHistory(AutoMoDeFsmHistory* pc_fsm_history);

			/*
			 * Class destructor.
			 */
			virtual ~AutoMoDeFsmHistory();

			/*
			 *
			 */
			void AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state, std::map<AutoMoDeCondition*, bool> map_transition_status);

			/*
			 *
			 */
			void AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state);

			/*
			 * Open the file where the history will be saved.
			 */
			void OpenFile();

		private:
			/*
			 * Path of the file where the history will be saved.
			 */
			std::string m_strPath;

			/*
			 * Content of the history.
			 */
			std::ofstream m_ofHistoryFile;

			/*
			 * Returns the path of the history file.
			 */
			const std::string& GetPath() const;
	};
}

#endif

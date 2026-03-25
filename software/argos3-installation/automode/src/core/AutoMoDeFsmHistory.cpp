/*
 * @file <src/core/AutoMoDeFsmHistory.cpp>
 *
 * @author Antoine Ligot - <aligot@ulb.ac.be>
 *
 * @package ARGoS3-AutoMoDe
 *
 * @license MIT License
 */

#include "AutoMoDeFsmHistory.h"

namespace argos {

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::AutoMoDeFsmHistory(const std::string& str_path) {
		m_strPath = str_path;
		OpenFile();
	}

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::AutoMoDeFsmHistory(AutoMoDeFsmHistory* pc_fsm_history) {
		m_strPath = pc_fsm_history->GetPath();
		OpenFile();
	}

	/****************************************/
	/****************************************/

	AutoMoDeFsmHistory::~AutoMoDeFsmHistory() {}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::OpenFile() {
		m_ofHistoryFile.open(m_strPath.c_str(), std::ofstream::out | std::ofstream::trunc);
		if(m_ofHistoryFile.fail()) {
			THROW_ARGOSEXCEPTION("Error opening file \"" << m_strPath);
		}
	}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state, std::map<AutoMoDeCondition*, bool> map_transition_status) {
		std::stringstream ssInput;
		ssInput << "--t " << un_time_step << " ";
		ssInput << "--s" << pc_current_state->GetIndex() << " " << pc_current_state->GetIdentifier() << " ";

		std::map<AutoMoDeCondition*, bool>::iterator it;
		for (it = map_transition_status.begin(); it != map_transition_status.end(); ++it) {
			ssInput << "--c" << (it->first)->GetIndex() << " "  << (it->first)->GetIdentifier() << " " << (it->second) << " ";
		}

		m_ofHistoryFile << ssInput.str() << std::endl;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeFsmHistory::AddTimeStep(UInt32 un_time_step, AutoMoDeBehaviour* pc_current_state) {
		std::stringstream ssInput;
		ssInput << "--t " << un_time_step << " ";
		ssInput << "--s" << pc_current_state->GetIndex() << " " << pc_current_state->GetIdentifier() << " ";

		m_ofHistoryFile << ssInput.str() << std::endl;
	}

	/****************************************/
	/****************************************/

	const std::string& AutoMoDeFsmHistory::GetPath() const{
		return m_strPath;
	}
}

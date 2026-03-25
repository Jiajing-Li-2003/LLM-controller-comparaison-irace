/**
  * @file <src/modules/AutoMoDeCondition.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#include "AutoMoDeCondition.h"

namespace argos {

	/****************************************/
	/****************************************/

	const std::string AutoMoDeCondition::GetDOTDescription() {
		std::stringstream ss;
		ss << m_strLabel;
		if (!m_mapParameters.empty()) {
			std::map<std::string, Real>::iterator it;
			for (it = m_mapParameters.begin(); it != m_mapParameters.end(); it++) {
				ss << "\\n" << it->first << "=" << it->second ;
			}
		}
		return ss.str();
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::AddParameter(const std::string& str_identifier, const Real& f_value) {
		m_mapParameters.insert(std::pair<std::string, Real>(str_identifier, f_value));
	}

	/****************************************/
	/****************************************/

	const UInt32& AutoMoDeCondition::GetOrigin() const {
		return m_unFromBehaviourIndex;
	}

	/****************************************/
	/****************************************/

	const UInt32& AutoMoDeCondition::GetExtremity() const {
		return m_unToBehaviourIndex;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::SetOrigin(const UInt32& un_from) {
		m_unFromBehaviourIndex = un_from;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::SetExtremity(const UInt32& un_to) {
		m_unToBehaviourIndex = un_to;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::SetOriginAndExtremity(const UInt32& un_from, const UInt32& un_to) {
		m_unFromBehaviourIndex = un_from;
		m_unToBehaviourIndex = un_to;
	}

	/****************************************/
	/****************************************/

	const std::string& AutoMoDeCondition::GetLabel() const {
		return m_strLabel;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::SetIndex(const UInt32& un_index) {
		m_unIndex = un_index;
	}

	/****************************************/
	/****************************************/

	const UInt32& AutoMoDeCondition::GetIndex() const {
		return m_unIndex;
	}

	/****************************************/
	/****************************************/

	void AutoMoDeCondition::SetIdentifier(const UInt32& un_id) {
		m_unIdentifier = un_id;
	}

	/****************************************/
	/****************************************/

	const UInt32& AutoMoDeCondition::GetIdentifier() const {
		return m_unIdentifier;
	}

	/****************************************/
	/****************************************/

	std::map<std::string, Real> AutoMoDeCondition::GetParameters() const {
		return m_mapParameters;
	}

  /****************************************/
	/****************************************/

  void AutoMoDeCondition::SetRobotDAO(EpuckDAO* pc_robot_dao) {
      m_pcRobotDAO = pc_robot_dao;
  }

	/****************************************/
	/****************************************/

  bool AutoMoDeCondition::EvaluateBernoulliProbability(const Real& f_probability) const {
		return m_pcRobotDAO->GetRandomNumberGenerator()->Bernoulli(f_probability);
	}

  /****************************************/
  /****************************************/
  // Return the color parameter
  CColor AutoMoDeCondition::GetColorParameter(const UInt32& un_value) {
      CColor cColorParameter;
      switch(un_value){
      case 0:
          cColorParameter = CColor::BLACK;
          break;
      case 1:
          cColorParameter = CColor::RED;
          break;
      case 2:
          cColorParameter = CColor::GREEN;
          break;
      case 3:
          cColorParameter = CColor::BLUE;
          break;
      case 4:
          cColorParameter = CColor::YELLOW;
          break;
      case 5:
          cColorParameter = CColor::MAGENTA;
          break;
      case 6:
          cColorParameter = CColor::CYAN;
          break;
      default:
          cColorParameter = CColor::BLACK;
      }
      return cColorParameter;
  }

}

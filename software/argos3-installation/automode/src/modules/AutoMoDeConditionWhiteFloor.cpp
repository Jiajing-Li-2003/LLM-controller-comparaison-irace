/**
  * @file <src/modules/AutoMoDeConditionInvertedWhiteFloor.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

 #include "AutoMoDeConditionWhiteFloor.h"

 namespace argos {

	/****************************************/
	/****************************************/

	AutoMoDeConditionWhiteFloor::AutoMoDeConditionWhiteFloor() {
		m_strLabel = "WhiteFloor";
	}

	/****************************************/
	/****************************************/

	AutoMoDeConditionWhiteFloor::~AutoMoDeConditionWhiteFloor() {}

	/****************************************/
	/****************************************/

	bool AutoMoDeConditionWhiteFloor::Verify() {
    if (m_pcRobotDAO->GetGroundReading() >= m_fGroundThreshold) {
      return EvaluateBernoulliProbability(m_fProbability);
    }
    else {
      return false;
    }
	}

	/****************************************/
	/****************************************/

	AutoMoDeConditionWhiteFloor::AutoMoDeConditionWhiteFloor(AutoMoDeConditionWhiteFloor* pc_condition) {
		m_strLabel = pc_condition->GetLabel();
		m_unIndex = pc_condition->GetIndex();
		m_unIdentifier = pc_condition->GetIndex();
		m_unFromBehaviourIndex = pc_condition->GetOrigin();
		m_unToBehaviourIndex = pc_condition->GetExtremity();
		m_mapParameters = pc_condition->GetParameters();
    Init();
	}

	/****************************************/
	/****************************************/

	AutoMoDeConditionWhiteFloor* AutoMoDeConditionWhiteFloor::Clone() {
		return new AutoMoDeConditionWhiteFloor(this);
	}

	/****************************************/
	/****************************************/

	void AutoMoDeConditionWhiteFloor::Init() {
		m_fGroundThreshold = 0.95;
		std::map<std::string, Real>::iterator it = m_mapParameters.find("p");
		if (it != m_mapParameters.end()) {
			m_fProbability = it->second;
		} else {
			LOGERR << "[FATAL] Missing parameter for the following condition:" << m_strLabel << std::endl;
			THROW_ARGOSEXCEPTION("Missing Parameter");
		}
	}

	/****************************************/
	/****************************************/

	void AutoMoDeConditionWhiteFloor::Reset() {}

 }

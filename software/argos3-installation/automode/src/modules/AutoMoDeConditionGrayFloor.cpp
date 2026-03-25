/**
  * @file <src/modules/AutoMoDeConditionGrayFloor.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

 #include "AutoMoDeConditionGrayFloor.h"

 namespace argos {

  /****************************************/
  /****************************************/

	AutoMoDeConditionGrayFloor::AutoMoDeConditionGrayFloor() {
		m_strLabel = "GrayFloor";
	}

  /****************************************/
  /****************************************/

	AutoMoDeConditionGrayFloor::~AutoMoDeConditionGrayFloor() {}

  /****************************************/
  /****************************************/

	AutoMoDeConditionGrayFloor::AutoMoDeConditionGrayFloor(AutoMoDeConditionGrayFloor* pc_condition) {
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

	AutoMoDeConditionGrayFloor* AutoMoDeConditionGrayFloor::Clone() {
		return new AutoMoDeConditionGrayFloor(this);
	}

  /****************************************/
  /****************************************/

	bool AutoMoDeConditionGrayFloor::Verify() {
    if (m_fGroundThresholdRange.WithinMinBoundExcludedMaxBoundExcluded(m_pcRobotDAO->GetGroundReading())) {
      return EvaluateBernoulliProbability(m_fProbability);
    }
    else {
      return false;
    }
	}

  /****************************************/
  /****************************************/

	void AutoMoDeConditionGrayFloor::Init() {
		m_fGroundThresholdRange.Set(0.1, 0.95);
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

	void AutoMoDeConditionGrayFloor::Reset() {}

 }

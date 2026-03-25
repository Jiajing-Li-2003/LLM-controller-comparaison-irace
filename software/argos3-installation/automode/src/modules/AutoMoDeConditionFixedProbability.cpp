/**
  * @file <src/modules/AutoMoDeConditionFixedProbability.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

 #include "AutoMoDeConditionFixedProbability.h"

 namespace argos {

  /****************************************/
  /****************************************/

	AutoMoDeConditionFixedProbability::AutoMoDeConditionFixedProbability() {
		m_strLabel = "FixedProbability";
	}

  /****************************************/
  /****************************************/

	AutoMoDeConditionFixedProbability::~AutoMoDeConditionFixedProbability() {}

  /****************************************/
  /****************************************/

	AutoMoDeConditionFixedProbability::AutoMoDeConditionFixedProbability(AutoMoDeConditionFixedProbability* pc_condition) {
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

	AutoMoDeConditionFixedProbability* AutoMoDeConditionFixedProbability::Clone() {
		return new AutoMoDeConditionFixedProbability(this);
	}

  /****************************************/
  /****************************************/

	void AutoMoDeConditionFixedProbability::Init() {
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

	bool AutoMoDeConditionFixedProbability::Verify() {
		return EvaluateBernoulliProbability(m_fProbability);
	}

  /****************************************/
  /****************************************/

	void AutoMoDeConditionFixedProbability::Reset() {
    Init();
  }

 }

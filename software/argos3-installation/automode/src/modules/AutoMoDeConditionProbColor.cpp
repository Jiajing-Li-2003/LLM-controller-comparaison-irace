/**
  * @file <src/modules/AutoMoDeConditionColor.cpp>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

 #include "AutoMoDeConditionProbColor.h"

 namespace argos {

  /****************************************/
  /****************************************/

    AutoMoDeConditionProbColor::AutoMoDeConditionProbColor() {
        m_strLabel = "ProbColor";
	}

  /****************************************/
  /****************************************/

    AutoMoDeConditionProbColor::~AutoMoDeConditionProbColor() {}

  /****************************************/
  /****************************************/

    AutoMoDeConditionProbColor::AutoMoDeConditionProbColor(AutoMoDeConditionProbColor* pc_condition) {
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

    void AutoMoDeConditionProbColor::Init() {
        std::map<std::string, Real>::iterator it = m_mapParameters.find("l");
        if (it != m_mapParameters.end()) {
            m_cColorParameter = GetColorParameter(it->second);
        } else {
            LOGERR << "[FATAL] Missing parameter for the following condition:" << m_strLabel << std::endl;
            THROW_ARGOSEXCEPTION("Missing Parameter");
        }
        it = m_mapParameters.find("p");
        if (it != m_mapParameters.end()) {
            m_fProbability = it->second;
        } else {
            LOGERR << "[FATAL] Missing parameter for the following condition:" << m_strLabel << std::endl;
            THROW_ARGOSEXCEPTION("Missing Parameter");
        }
    }

  /****************************************/
  /****************************************/

    AutoMoDeConditionProbColor* AutoMoDeConditionProbColor::Clone() {
        return new AutoMoDeConditionProbColor(this);
	}

  /****************************************/
  /****************************************/

    bool AutoMoDeConditionProbColor::Verify() {
        CCI_EPuckOmnidirectionalCameraSensor::SReadings sReadings = m_pcRobotDAO->GetCameraInput();
        CCI_EPuckOmnidirectionalCameraSensor::TBlobList::iterator it;
        bool bColorPerceived = false;

        for (it = sReadings.BlobList.begin(); it != sReadings.BlobList.end(); it++) {
            if ((*it)->Color == m_cColorParameter && (*it)->Distance >= 6.0) {
                bColorPerceived = true;
                break;
            }
        }

        if (bColorPerceived){
            return EvaluateBernoulliProbability(m_fProbability);
        }

    return false;
 }

  /****************************************/
  /****************************************/

    void AutoMoDeConditionProbColor::Reset() {
    Init();
  }

 }

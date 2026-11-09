/**
  * @file <src/modules/AutoMoDeBehaviourZigZag.cpp>
  *
  * @brief Comportement ZigZag (code B=11)
  * Avance tout droit pendant zfr steps, tourne à gauche brièvement,
  * avance, tourne à droite, etc. Crée un balayage en zigzag.
  */

#include "AutoMoDeBehaviourZigZag.h"

namespace argos {

    AutoMoDeBehaviourZigZag::AutoMoDeBehaviourZigZag() {
        m_strLabel = "ZigZag";
    }

    AutoMoDeBehaviourZigZag::AutoMoDeBehaviourZigZag(AutoMoDeBehaviourZigZag* pc_behaviour) {
        m_strLabel      = pc_behaviour->GetLabel();
        m_bLocked       = pc_behaviour->IsLocked();
        m_bOperational  = pc_behaviour->IsOperational();
        m_unIndex       = pc_behaviour->GetIndex();
        m_unIdentifier  = pc_behaviour->GetIdentifier();
        m_mapParameters = pc_behaviour->GetParameters();
        Init();
    }

    AutoMoDeBehaviourZigZag::~AutoMoDeBehaviourZigZag() {}

    AutoMoDeBehaviourZigZag* AutoMoDeBehaviourZigZag::Clone() {
        return new AutoMoDeBehaviourZigZag(this);
    }

    void AutoMoDeBehaviourZigZag::Init() {
        m_unStepCount         = 0;
        m_bTurningLeft        = true;
        m_fProximityThreshold = 0.1;

        std::map<std::string, Real>::iterator it = m_mapParameters.find("zfr");
        if (it != m_mapParameters.end()) {
            m_unZigZagFrequency = (UInt32)it->second;
        } else {
            LOGERR << "[FATAL] Missing parameter for the following behaviour:" << m_strLabel << std::endl;
            THROW_ARGOSEXCEPTION("Missing Parameter");
        }

        it = m_mapParameters.find("cle");
        if (it != m_mapParameters.end()) {
            m_cColorEmiterParameter = GetColorParameter(it->second, true);
        } else {
            LOGERR << "[FATAL] Missing parameter for the following behaviour:" << m_strLabel << std::endl;
            THROW_ARGOSEXCEPTION("Missing Parameter");
        }
    }

    void AutoMoDeBehaviourZigZag::ControlStep() {
        // Évitement d'obstacle prioritaire
        CCI_EPuckProximitySensor::SReading sProx = m_pcRobotDAO->GetProximityReading();
        if (sProx.Value >= m_fProximityThreshold) {
            // Tourner dans la direction opposée à l'obstacle
            CRadians cAngle = sProx.Angle.SignedNormalize();
            if (cAngle.GetValue() < 0) {
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(m_pcRobotDAO->GetMaxVelocity(),
                            -m_pcRobotDAO->GetMaxVelocity()));
            } else {
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(-m_pcRobotDAO->GetMaxVelocity(),
                              m_pcRobotDAO->GetMaxVelocity()));
            }
            // Inverser la direction du zigzag au prochain cycle
            m_bTurningLeft = !m_bTurningLeft;
            m_pcRobotDAO->SetLEDsColor(m_cColorEmiterParameter);
            m_bLocked = false;
            return;
        }

        m_unStepCount++;
        UInt32 unPhase = m_unStepCount % (m_unZigZagFrequency * 2);

        if (unPhase < m_unZigZagFrequency) {
            // Phase avancer tout droit
            m_pcRobotDAO->SetWheelsVelocity(
                CVector2(m_pcRobotDAO->GetMaxVelocity(),
                         m_pcRobotDAO->GetMaxVelocity()));
        } else {
            // Phase virer (alternance gauche/droite selon le cycle)
            UInt32 unCycle = m_unStepCount / (m_unZigZagFrequency * 2);
            if (unCycle % 2 == 0) {
                // Virage à gauche
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(-m_pcRobotDAO->GetMaxVelocity() * 0.3,
                              m_pcRobotDAO->GetMaxVelocity()));
            } else {
                // Virage à droite
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(m_pcRobotDAO->GetMaxVelocity(),
                            -m_pcRobotDAO->GetMaxVelocity() * 0.3));
            }
        }

        m_pcRobotDAO->SetLEDsColor(m_cColorEmiterParameter);
        m_bLocked = false;
    }

    void AutoMoDeBehaviourZigZag::Reset() {
        m_bOperational = false;
        Init();
        ResumeStep();
    }

    void AutoMoDeBehaviourZigZag::ResumeStep() {
        m_bOperational = true;
    }
}

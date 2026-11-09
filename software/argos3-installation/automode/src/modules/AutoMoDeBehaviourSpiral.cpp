/**
  * @file <src/modules/AutoMoDeBehaviourSpiral.cpp>
  *
  * @brief Comportement Spirale (code B=6)
  * Le robot avance tout droit pendant un moment croissant,
  * puis tourne à droite, créant une trajectoire en spirale.
  * Cela couvre l'arène plus systématiquement que la marche aléatoire.
  *
  * Paramètres :
  *   --spdI [1..10]  : vitesse/rayon de spirale (défaut: 5)
  *   --cleI [0..6]   : couleur LED
  */

#include "AutoMoDeBehaviourSpiral.h"

namespace argos {

    AutoMoDeBehaviourSpiral::AutoMoDeBehaviourSpiral() {
        m_strLabel = "Spiral";
    }

    AutoMoDeBehaviourSpiral::AutoMoDeBehaviourSpiral(AutoMoDeBehaviourSpiral* pc_behaviour) {
        m_strLabel       = pc_behaviour->GetLabel();
        m_bLocked        = pc_behaviour->IsLocked();
        m_bOperational   = pc_behaviour->IsOperational();
        m_unIndex        = pc_behaviour->GetIndex();
        m_unIdentifier   = pc_behaviour->GetIdentifier();
        m_mapParameters  = pc_behaviour->GetParameters();
        Init();
    }

    AutoMoDeBehaviourSpiral::~AutoMoDeBehaviourSpiral() {}

    AutoMoDeBehaviourSpiral* AutoMoDeBehaviourSpiral::Clone() {
        return new AutoMoDeBehaviourSpiral(this);
    }

    void AutoMoDeBehaviourSpiral::Init() {
        m_unStepCount       = 0;
        m_unTurnSteps       = 0;
        m_bTurning          = false;
        m_fProximityThreshold = 0.1;

        std::map<std::string, Real>::iterator it = m_mapParameters.find("spd");
        if (it != m_mapParameters.end()) {
            m_fSpiralSpeed = it->second;
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

        // La période de la spirale dépend de la vitesse (plus lent = spirale plus large)
        m_unSpiralPeriod = (UInt32)(10.0 * m_fSpiralSpeed);
    }

    void AutoMoDeBehaviourSpiral::ControlStep() {

        // Si un obstacle est détecté, éviter
        CCI_EPuckProximitySensor::SReading sProx = m_pcRobotDAO->GetProximityReading();
        if (sProx.Value >= m_fProximityThreshold) {
            // Tourner à l'opposé de l'obstacle
            CRadians cAngle = sProx.Angle.SignedNormalize();
            if (cAngle.GetValue() < 0) {
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(m_pcRobotDAO->GetMaxVelocity(),
                             -m_pcRobotDAO->GetMaxVelocity() * 0.5));
            } else {
                m_pcRobotDAO->SetWheelsVelocity(
                    CVector2(-m_pcRobotDAO->GetMaxVelocity() * 0.5,
                              m_pcRobotDAO->GetMaxVelocity()));
            }
            m_pcRobotDAO->SetLEDsColor(m_cColorEmiterParameter);
            m_bLocked = false;
            return;
        }

        m_unStepCount++;

        if (!m_bTurning) {
            // Phase avancer tout droit — durée augmente avec le temps (spirale)
            m_pcRobotDAO->SetWheelsVelocity(
                CVector2(m_pcRobotDAO->GetMaxVelocity(),
                         m_pcRobotDAO->GetMaxVelocity()));

            // Après m_unSpiralPeriod * (tour actuel) steps, tourner
            UInt32 unCurrentPeriod = m_unSpiralPeriod + (m_unStepCount / 20);
            if (m_unStepCount % unCurrentPeriod == 0) {
                m_bTurning    = true;
                m_unTurnSteps = (UInt32)(5.0 + m_fSpiralSpeed);
            }
        } else {
            // Phase tourner à droite (spirale dans le sens horaire)
            m_pcRobotDAO->SetWheelsVelocity(
                CVector2(m_pcRobotDAO->GetMaxVelocity(),
                        -m_pcRobotDAO->GetMaxVelocity() * 0.8));

            m_unTurnSteps--;
            if (m_unTurnSteps == 0) {
                m_bTurning = false;
            }
        }

        m_pcRobotDAO->SetLEDsColor(m_cColorEmiterParameter);
        m_bLocked = false;
    }

    void AutoMoDeBehaviourSpiral::Reset() {
        m_bOperational = false;
        Init();
        ResumeStep();
    }

    void AutoMoDeBehaviourSpiral::ResumeStep() {
        m_bOperational = true;
    }
}

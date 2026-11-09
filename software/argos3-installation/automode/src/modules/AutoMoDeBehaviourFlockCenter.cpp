/**
  * @file <src/modules/AutoMoDeBehaviourFlockCenter.cpp>
  *
  * @brief Comportement FlockCenter (code B=10)
  * Va vers le centre de masse de tous les voisins détectés par RAB.
  * Si aucun voisin : explore aléatoirement.
  */

#include "AutoMoDeBehaviourFlockCenter.h"

namespace argos {

    AutoMoDeBehaviourFlockCenter::AutoMoDeBehaviourFlockCenter() {
        m_strLabel = "FlockCenter";
    }

    AutoMoDeBehaviourFlockCenter::AutoMoDeBehaviourFlockCenter(AutoMoDeBehaviourFlockCenter* pc_behaviour) {
        m_strLabel      = pc_behaviour->GetLabel();
        m_bLocked       = pc_behaviour->IsLocked();
        m_bOperational  = pc_behaviour->IsOperational();
        m_unIndex       = pc_behaviour->GetIndex();
        m_unIdentifier  = pc_behaviour->GetIdentifier();
        m_mapParameters = pc_behaviour->GetParameters();
        Init();
    }

    AutoMoDeBehaviourFlockCenter::~AutoMoDeBehaviourFlockCenter() {}

    AutoMoDeBehaviourFlockCenter* AutoMoDeBehaviourFlockCenter::Clone() {
        return new AutoMoDeBehaviourFlockCenter(this);
    }

    void AutoMoDeBehaviourFlockCenter::Init() {
        std::map<std::string, Real>::iterator it = m_mapParameters.find("fc");
        if (it != m_mapParameters.end()) {
            m_fFlockParameter = it->second;
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

    void AutoMoDeBehaviourFlockCenter::ControlStep() {
        // Récupérer tous les messages RAB des voisins
        const CCI_EPuckRangeAndBearingSensor::TPackets& tPackets =
            m_pcRobotDAO->GetRangeAndBearingMessages();

        CVector2 sCenterVector(0, CRadians::ZERO);
        CVector2 sProxVector(0, CRadians::ZERO);
        CVector2 sResultVector(0, CRadians::ZERO);

        if (!tPackets.empty()) {
            // Calculer le vecteur moyen vers tous les voisins (centre de masse)
            Real fSumX = 0, fSumY = 0;
            UInt32 unCount = 0;

            for (const CCI_EPuckRangeAndBearingSensor::SReceivedPacket* pcPacket : tPackets) {
                if (pcPacket->Range > 0) {
                    CVector2 v(pcPacket->Range, pcPacket->Bearing);
                    fSumX += v.GetX();
                    fSumY += v.GetY();
                    unCount++;
                }
            }

            if (unCount > 0) {
                // Vecteur vers le centre de masse, pondéré par m_fFlockParameter
                sCenterVector = CVector2(
                    (fSumX / unCount) * m_fFlockParameter,
                    (fSumY / unCount) * m_fFlockParameter
                );
            }
        }

        // Évitement des obstacles (proximité)
        sProxVector = CVector2(
            m_pcRobotDAO->GetProximityReading().Value,
            m_pcRobotDAO->GetProximityReading().Angle
        );

        if (sCenterVector.Length() > 0.01) {
            // Aller vers le centre en évitant les obstacles
            sResultVector = sCenterVector - 5.0 * sProxVector;
        } else {
            // Pas de voisins : avancer tout droit
            sResultVector = CVector2(1, CRadians::ZERO) - 5.0 * sProxVector;
        }

        if (sResultVector.Length() < 0.1) {
            sResultVector = CVector2(1, CRadians::ZERO);
        }

        m_pcRobotDAO->SetWheelsVelocity(ComputeWheelsVelocityFromVector(sResultVector));
        m_pcRobotDAO->SetLEDsColor(m_cColorEmiterParameter);
        m_bLocked = false;
    }

    void AutoMoDeBehaviourFlockCenter::Reset() {
        m_bOperational = false;
        ResumeStep();
    }

    void AutoMoDeBehaviourFlockCenter::ResumeStep() {
        m_bOperational = true;
    }
}

/**
  * @file <src/modules/AutoMoDeBehaviourSpiral.h>
  *
  * @brief Comportement Spirale (code B=6)
  * Le robot tourne en spirale croissante pour couvrir l'arène
  * plus efficacement que l'exploration aléatoire.
  *
  * Paramètres obligatoires :
  *   --spdI [1..10]  : vitesse de la spirale
  *   --cleI [0..6]   : couleur LED émise
  */

#ifndef AUTOMODE_BEHAVIOUR_SPIRAL_H
#define AUTOMODE_BEHAVIOUR_SPIRAL_H

#include "AutoMoDeBehaviour.h"

namespace argos {
    class AutoMoDeBehaviourSpiral: public AutoMoDeBehaviour {
        public:
            AutoMoDeBehaviourSpiral();
            AutoMoDeBehaviourSpiral(AutoMoDeBehaviourSpiral* pc_behaviour);
            virtual ~AutoMoDeBehaviourSpiral();

            virtual void ControlStep();
            virtual void Reset();
            virtual void ResumeStep();
            virtual void Init();

            virtual AutoMoDeBehaviourSpiral* Clone();

        private:
            Real m_fSpiralSpeed;
            UInt32 m_unStepCount;
            UInt32 m_unSpiralPeriod;
            UInt32 m_unTurnSteps;
            bool m_bTurning;
            Real m_fProximityThreshold;
            CColor m_cColorEmiterParameter;
    };
}

#endif

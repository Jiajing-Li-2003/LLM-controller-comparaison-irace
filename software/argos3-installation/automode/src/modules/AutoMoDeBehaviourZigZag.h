/**
  * @file <src/modules/AutoMoDeBehaviourZigZag.h>
  *
  * @brief Comportement ZigZag (code B=11)
  * Le robot se déplace en zigzag régulier, alternant gauche/droite.
  * Utile pour patrouiller une zone en balayage, comme un robot aspirateur.
  * Contrairement à l'exploration aléatoire, le zigzag est déterministe
  * et couvre la zone de façon plus uniforme.
  *
  * Paramètres obligatoires :
  *   --zfrI [1..20]  : fréquence du zigzag (nb de steps entre chaque virage)
  *   --cleI [0..6]   : couleur LED émise
  */

#ifndef AUTOMODE_BEHAVIOUR_ZIGZAG_H
#define AUTOMODE_BEHAVIOUR_ZIGZAG_H

#include "AutoMoDeBehaviour.h"

namespace argos {
    class AutoMoDeBehaviourZigZag: public AutoMoDeBehaviour {
        public:
            AutoMoDeBehaviourZigZag();
            AutoMoDeBehaviourZigZag(AutoMoDeBehaviourZigZag* pc_behaviour);
            virtual ~AutoMoDeBehaviourZigZag();

            virtual void ControlStep();
            virtual void Reset();
            virtual void ResumeStep();
            virtual void Init();

            virtual AutoMoDeBehaviourZigZag* Clone();

        private:
            UInt32 m_unZigZagFrequency;
            UInt32 m_unStepCount;
            bool m_bTurningLeft;
            Real m_fProximityThreshold;
            CColor m_cColorEmiterParameter;
    };
}

#endif

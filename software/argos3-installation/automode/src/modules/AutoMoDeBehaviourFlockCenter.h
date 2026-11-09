/**
  * @file <src/modules/AutoMoDeBehaviourFlockCenter.h>
  *
  * @brief Comportement FlockCenter (code B=10)
  * Le robot calcule le centre de masse de ses voisins (via RAB)
  * et s'y dirige. Crée un comportement d'agrégation au centre du groupe.
  * Différent de l'Attraction qui va vers le voisin le plus proche —
  * ici le robot va vers la moyenne de tous les voisins.
  *
  * Paramètres obligatoires :
  *   --fcI [1..10]   : force de déplacement vers le centre
  *   --cleI [0..6]   : couleur LED émise
  */

#ifndef AUTOMODE_BEHAVIOUR_FLOCKCENTER_H
#define AUTOMODE_BEHAVIOUR_FLOCKCENTER_H

#include "AutoMoDeBehaviour.h"

namespace argos {
    class AutoMoDeBehaviourFlockCenter: public AutoMoDeBehaviour {
        public:
            AutoMoDeBehaviourFlockCenter();
            AutoMoDeBehaviourFlockCenter(AutoMoDeBehaviourFlockCenter* pc_behaviour);
            virtual ~AutoMoDeBehaviourFlockCenter();

            virtual void ControlStep();
            virtual void Reset();
            virtual void ResumeStep();
            virtual void Init();

            virtual AutoMoDeBehaviourFlockCenter* Clone();

        private:
            Real m_fFlockParameter;
            CColor m_cColorEmiterParameter;
    };
}

#endif

/**
  * @file <src/modules/AutoMoDeBehaviourRepulsionColor.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_GOAWAY_COLOR_H
#define AUTOMODE_BEHAVIOUR_GOAWAY_COLOR_H

#include "AutoMoDeBehaviour.h"

namespace argos {
    class AutoMoDeBehaviourGoAwayColor: public AutoMoDeBehaviour {
		public:
            AutoMoDeBehaviourGoAwayColor();
            AutoMoDeBehaviourGoAwayColor(AutoMoDeBehaviourGoAwayColor* pc_behaviour);
            virtual ~AutoMoDeBehaviourGoAwayColor();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

            virtual AutoMoDeBehaviourGoAwayColor* Clone();

		private:
            CColor m_cColorEmiterParameter;
            CColor m_cColorReceiverParameter;
            Real m_unRepulsionParameter;
	};
}


#endif

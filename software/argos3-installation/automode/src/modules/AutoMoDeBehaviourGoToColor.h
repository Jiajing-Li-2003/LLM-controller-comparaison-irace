/**
  * @file <src/modules/AutoMoDeBehaviourAttractionColor.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_GOTO_COLOR_H
#define AUTOMODE_BEHAVIOUR_GOTO_COLOR_H

#include "AutoMoDeBehaviour.h"

namespace argos {
    class AutoMoDeBehaviourGoToColor: public AutoMoDeBehaviour {
		public:
            AutoMoDeBehaviourGoToColor();
            AutoMoDeBehaviourGoToColor(AutoMoDeBehaviourGoToColor* pc_behaviour);
            virtual ~AutoMoDeBehaviourGoToColor();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

            virtual AutoMoDeBehaviourGoToColor* Clone();

        private:
            Real m_fDistanceWeightParameter;
            CColor m_cColorEmiterParameter;
            CColor m_cColorReceiverParameter;
            Real m_unAttractionParameter;
	};
}

#endif

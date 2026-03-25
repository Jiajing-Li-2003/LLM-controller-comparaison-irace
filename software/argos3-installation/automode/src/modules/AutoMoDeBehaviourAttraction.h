/**
  * @file <src/modules/AutoMoDeBehaviourAttraction.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_ATTRACTION_H
#define AUTOMODE_BEHAVIOUR_ATTRACTION_H

#include "AutoMoDeBehaviour.h"

namespace argos {
	class AutoMoDeBehaviourAttraction: public AutoMoDeBehaviour {
		public:
			AutoMoDeBehaviourAttraction();
			AutoMoDeBehaviourAttraction(AutoMoDeBehaviourAttraction* pc_behaviour);
			virtual ~AutoMoDeBehaviourAttraction();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

			virtual AutoMoDeBehaviourAttraction* Clone();

		private:
			UInt8 m_unAttractionParameter;
            CColor m_cColorEmiterParameter;
	};
}

#endif

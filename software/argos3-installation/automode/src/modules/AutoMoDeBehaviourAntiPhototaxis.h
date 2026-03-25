/**
  * @file <src/modules/AutoMoDeBehaviourAntiPhototaxis.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_ANTI_PHOTOTAXIS_H
#define AUTOMODE_BEHAVIOUR_ANTI_PHOTOTAXIS_H

#include "AutoMoDeBehaviour.h"

namespace argos {
	class AutoMoDeBehaviourAntiPhototaxis: public AutoMoDeBehaviour {
		public:
			AutoMoDeBehaviourAntiPhototaxis();
			AutoMoDeBehaviourAntiPhototaxis(AutoMoDeBehaviourAntiPhototaxis* pc_behaviour);
			virtual ~AutoMoDeBehaviourAntiPhototaxis();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

			virtual AutoMoDeBehaviourAntiPhototaxis* Clone();
	};
}

#endif

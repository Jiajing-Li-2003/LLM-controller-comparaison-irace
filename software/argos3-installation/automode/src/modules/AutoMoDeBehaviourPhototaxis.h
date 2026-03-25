/**
  * @file <src/modules/AutoMoDeBehaviourPhototaxis.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_PHOTOTAXIS_H
#define AUTOMODE_BEHAVIOUR_PHOTOTAXIS_H

#include "AutoMoDeBehaviour.h"

namespace argos {
	class AutoMoDeBehaviourPhototaxis: public AutoMoDeBehaviour {
		public:
			AutoMoDeBehaviourPhototaxis();
			AutoMoDeBehaviourPhototaxis(AutoMoDeBehaviourPhototaxis* pc_behaviour);
			virtual ~AutoMoDeBehaviourPhototaxis();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

			virtual AutoMoDeBehaviourPhototaxis* Clone();
	};
}

#endif

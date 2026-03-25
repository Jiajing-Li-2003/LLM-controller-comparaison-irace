/**
  * @file <src/modules/AutoMoDeBehaviourRepulsion.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_BEHAVIOUR_REPULSION_H
#define AUTOMODE_BEHAVIOUR_REPULSION_H

#include "AutoMoDeBehaviour.h"

namespace argos {
	class AutoMoDeBehaviourRepulsion: public AutoMoDeBehaviour {
		public:
			AutoMoDeBehaviourRepulsion();
			AutoMoDeBehaviourRepulsion(AutoMoDeBehaviourRepulsion* pc_behaviour);
			virtual ~AutoMoDeBehaviourRepulsion();

			virtual void ControlStep();
			virtual void Reset();
			virtual void ResumeStep();
			virtual void Init();

			virtual AutoMoDeBehaviourRepulsion* Clone();

		private:
			UInt8 m_unRepulsionParameter;
            CColor m_cColorEmiterParameter;
	};
}


#endif

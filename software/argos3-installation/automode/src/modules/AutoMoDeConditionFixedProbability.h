/**
  * @file <src/modules/AutoMoDeConditionFixedProbability.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_FIXED_PROBABILITY_H
#define AUTOMODE_CONDITION_FIXED_PROBABILITY_H

#include "AutoMoDeCondition.h"

namespace argos {
	class AutoMoDeConditionFixedProbability: public AutoMoDeCondition {
		public:
			AutoMoDeConditionFixedProbability();
			virtual ~AutoMoDeConditionFixedProbability();

			AutoMoDeConditionFixedProbability(AutoMoDeConditionFixedProbability* pc_condition);
			virtual AutoMoDeConditionFixedProbability* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
			Real m_fProbability;
	};
}

#endif

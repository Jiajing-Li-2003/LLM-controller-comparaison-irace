/**
  * @file <src/modules/AutoMoDeConditionGrayFloor.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_GRAY_FLOOR_H
#define AUTOMODE_CONDITION_GRAY_FLOOR_H

#include "AutoMoDeCondition.h"

namespace argos {
	class AutoMoDeConditionGrayFloor: public AutoMoDeCondition {
		public:
			AutoMoDeConditionGrayFloor();
			virtual ~AutoMoDeConditionGrayFloor();

			AutoMoDeConditionGrayFloor(AutoMoDeConditionGrayFloor* pc_condition);
			virtual AutoMoDeConditionGrayFloor* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
			CRange<Real> m_fGroundThresholdRange;
			Real m_fProbability;
	};
}

#endif

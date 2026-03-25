/**
  * @file <src/modules/AutoMoDeConditionBlackFloor.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_BLACK_FLOOR_H
#define AUTOMODE_CONDITION_BLACK_FLOOR_H

#include "AutoMoDeCondition.h"

namespace argos {
	class AutoMoDeConditionBlackFloor: public AutoMoDeCondition {
		public:
			AutoMoDeConditionBlackFloor();
			virtual ~AutoMoDeConditionBlackFloor();

			AutoMoDeConditionBlackFloor(AutoMoDeConditionBlackFloor* pc_condition);
			virtual AutoMoDeConditionBlackFloor* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
			Real m_fGroundThreshold;
			Real m_fProbability;
	};
}

#endif

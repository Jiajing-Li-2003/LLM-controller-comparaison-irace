/**
  * @file <src/modules/AutoMoDeConditionNeighborsCount.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_NEIGHBORS_COUNT_H
#define AUTOMODE_CONDITION_NEIGHBORS_COUNT_H

#include "AutoMoDeCondition.h"

namespace argos {
	class AutoMoDeConditionNeighborsCount: public AutoMoDeCondition {
		public:
			AutoMoDeConditionNeighborsCount();
			virtual ~AutoMoDeConditionNeighborsCount();

			AutoMoDeConditionNeighborsCount(AutoMoDeConditionNeighborsCount* pc_condition);
			virtual AutoMoDeConditionNeighborsCount* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
			Real m_fParameterEta;
			UInt8 m_unParameterXi;
	};
}

#endif

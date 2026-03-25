/**
  * @file <src/modules/AutoMoDeConditionInvertedNeighborsCount.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_INVERTED_NEIGHBORS_COUNT_H
#define AUTOMODE_CONDITION_INVERTED_NEIGHBORS_COUNT_H

#include "AutoMoDeCondition.h"

namespace argos {
	class AutoMoDeConditionInvertedNeighborsCount: public AutoMoDeCondition {
		public:
			AutoMoDeConditionInvertedNeighborsCount();
			virtual ~AutoMoDeConditionInvertedNeighborsCount();

			AutoMoDeConditionInvertedNeighborsCount(AutoMoDeConditionInvertedNeighborsCount* pc_condition);
			virtual AutoMoDeConditionInvertedNeighborsCount* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
			Real m_fParameterEta;
			UInt8 m_unParameterXi;
	};
}

#endif

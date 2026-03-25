/**
  * @file <src/modules/AutoMoDeConditionColor.h>
  *
  * @author Antoine Ligot - <aligot@ulb.ac.be>
  *
  * @package ARGoS3-AutoMoDe
  *
  * @license MIT License
  */

#ifndef AUTOMODE_CONDITION_PROB_COLOR_H
#define AUTOMODE_CONDITION_PROB_COLOR_H

#include "AutoMoDeCondition.h"

namespace argos {
    class AutoMoDeConditionProbColor: public AutoMoDeCondition {
		public:
            AutoMoDeConditionProbColor();
            virtual ~AutoMoDeConditionProbColor();

            AutoMoDeConditionProbColor(AutoMoDeConditionProbColor* pc_condition);
            virtual AutoMoDeConditionProbColor* Clone();

			virtual bool Verify();
			virtual void Reset();
			virtual void Init();

		private:
            CColor m_cColorParameter;
			Real m_fProbability;
            Real m_fDistance;
	};
}

#endif

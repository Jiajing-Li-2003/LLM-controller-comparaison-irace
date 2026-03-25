/**
 * @file <argos3/plugins/simulator/physics_engines/dynamics2d/dynamics2d_box_model.h>
 *
 * @author Carlo Pinciroli - <ilpincy@gmail.com>
 */

#ifndef DYNAMICS2D_BLOCK_MODEL_H
#define DYNAMICS2D_BLOCK_MODEL_H

namespace argos {
   class CDynamics2DStretchableObjectModel;
   class CDynamics2DBlockModel;
}

#include <argos3/plugins/simulator/physics_engines/dynamics2d/dynamics2d_stretchable_object_model.h>
#include "block_entity.h"

namespace argos {

   class CDynamics2DBlockModel : public CDynamics2DStretchableObjectModel {

   public:

      CDynamics2DBlockModel(CDynamics2DEngine& c_engine,
                          CBlockEntity& c_entity);
      virtual ~CDynamics2DBlockModel() {}
   };

}

#endif

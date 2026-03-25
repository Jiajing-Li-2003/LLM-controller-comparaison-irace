/**
 * @file <argos3/plugins/robots/arena/simulator/arena_entity.h>
 *
 * @author David Garzon-Ramos - <dgarzonr@ulb.ac.be>
 */

#ifndef ARENA_ENTITY_H
#define ARENA_ENTITY_H

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/plugins/simulator/media/led_medium.h>
#include "wall_entity.h"

namespace argos {
   class CWallEntity;
   class CArenaEntity;
}

namespace argos {

   class CArenaEntity : public CComposableEntity {

   public:

       ENABLE_VTABLE();

      CArenaEntity(
                 const std::string& str_id,
                 const CVector3& c_position,
                 const CQuaternion& c_orientation,
                 const CVector3& c_size,
                 const std::string& str_led_medium,
                 UInt32 un_boxes,
                 UInt32 un_edges,
                 Real f_gap = 0.03,
                 Real f_mass = 1.0f);

      virtual void Init(TConfigurationNode& t_tree);
      virtual void Reset();
      virtual void Destroy();

      virtual std::string GetTypeDescription() const {
         return "arena";
      }

      void AddWall(CWallEntity& c_wall);

      inline CPositionalEntity& GetPositionalEntity() {
         return *m_pcPositionalEntity;
      }

      inline const CPositionalEntity& GetPositionalEntity() const {
         return *m_pcPositionalEntity;
      }

      inline const std::vector<CWallEntity*>& GetWalls() {
         return m_vWalls;
      }

      void PositionWalls();

      void SetArenaColor(CColor vColor);

      void SetWallColor(UInt32 unWallID, CColor vColor);

      void SetBoxColor(UInt32 unBoxID, UInt32 unWallID, CColor vColor);

      bool IsEven(UInt32 unNumber);

      Real InnerRadious();

   private:

      CPositionalEntity*          m_pcPositionalEntity;
      std::vector<CWallEntity*>  m_vWalls;
      CLEDMedium*               m_pcLEDMedium;
      CVector3                  m_cSize;
      Real                      m_fMass;
      Real                      m_fGap;
      UInt32                    m_unNumberBoxes;
      UInt32                    m_unNumberEdges;

   };

}

#endif

/**
 * @file <argos3/plugins/robots/arena/simulator/wall_entity.h>
 *
 * @author David Garzon-Ramos - <dgarzonr@ulb.ac.be>
 */

#ifndef WALL_ENTITY_H
#define WALL_ENTITY_H

#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/plugins/simulator/entities/box_entity.h>
#include <argos3/plugins/simulator/media/led_medium.h>
#include "arena_entity.h"
#include "block_entity.h"

namespace argos {
   class CBlockEntity;
   class CWallEntity;
   class CArenaEntity;
}

namespace argos {

   class CWallEntity : public CComposableEntity {

   public:

       ENABLE_VTABLE();

      CWallEntity(CArenaEntity* pc_parent,
                 const std::string& str_id,
                 const CVector3& c_position,
                 const CQuaternion& c_orientation,
                 const CVector3& c_size,
                 const std::string& str_led_medium,
                 UInt32 un_boxes,
                 Real f_gap = 0.03,
                 Real f_mass = 1.0f);

      //~CWallEntity();

      virtual void Init(TConfigurationNode& t_tree);
      virtual void Reset();
      virtual void Destroy();

      virtual std::string GetTypeDescription() const {
         return "wall";
      }

      inline CPositionalEntity& GetPositionalEntity() {
         return *m_pcPositionalEntity;
      }

      inline const CPositionalEntity& GetPositionalEntity() const {
         return *m_pcPositionalEntity;
      }

      void AddBlock(CBlockEntity& c_block);

      void PositionBlocks();

      bool IsEven(UInt32 unNumber);

      inline const std::vector<CBlockEntity*>& GetBlocks() {
         return m_vBoxes;
      }

   private:

      CPositionalEntity*          m_pcPositionalEntity;
      std::vector<CBlockEntity*>  m_vBoxes;
      CLEDMedium*               m_pcLEDMedium;
      CVector3                  m_cSize;
      Real                      m_fMass;
      Real                      m_fGap;
      UInt32                    m_unNumberBoxes;

   };

}

#endif

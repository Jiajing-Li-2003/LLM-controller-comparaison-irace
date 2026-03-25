/**
 * @file <argos3/plugins/robots/arena/simulator/wall_entity.cpp>
 *
 * @author David Garzon-Ramos - <dgarzonr@ulb.ac.be>
 */

#include "wall_entity.h"

namespace argos {


   /****************************************/
   /****************************************/

   CWallEntity::CWallEntity(CArenaEntity* pc_parent,
                              const std::string& str_id,
                              const CVector3& c_position,
                              const CQuaternion& c_orientation,
                              const CVector3& c_size,
                              const std::string& str_led_medium,
                              UInt32 un_boxes,
                              Real f_gap,
                              Real f_mass) :
      CComposableEntity(NULL, str_id){
      try {
         /*
          * Create and init components
          */
         /* Posicional entity */
         m_pcPositionalEntity = new CPositionalEntity(this, "pose_0", c_position, c_orientation);

         m_pcLEDMedium = &CSimulator::GetInstance().GetMedium<CLEDMedium>(str_led_medium);
         m_cSize = c_size;
         m_fMass = f_mass;
         m_fGap = f_gap;
         m_unNumberBoxes = un_boxes;

         AddComponent(*m_pcPositionalEntity);

         PositionBlocks();

         /* Update components */
         UpdateComponents();
      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize entity \"" << GetId() << "\".", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CWallEntity::Init(TConfigurationNode& t_tree) {
      try {

           /*
             * Init parent
             */
           CComposableEntity::Init(t_tree);

         UpdateComponents();

      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize entity \"" << GetId() << "\".", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CWallEntity::Reset() {
      /* Reset all components */
      CComposableEntity::Reset();
      /* Update components */
      UpdateComponents();
   }

   /****************************************/
   /****************************************/

   void CWallEntity::Destroy() {

       CComposableEntity::Destroy();
   }

   /****************************************/
   /****************************************/

   void CWallEntity::AddBlock(CBlockEntity& c_block) {

      Real fFirstLED = (m_fGap/2)-(m_cSize.GetY()/2);
      UInt32 unNumberLEDs = ceil((m_cSize.GetY())/m_fGap);

      for (UInt32 i=0; i < unNumberLEDs; i++){
          Real unPosition = fFirstLED + i*m_fGap;
          //c_block.AddLED(CVector3(0.0125,  unPosition, 0.067), CColor::BLACK);
          c_block.AddLED(CVector3(0.0075,  unPosition, 0.067), CColor::BLACK);
      }

      c_block.Enable();
      c_block.GetLEDEquippedEntity().AddToMedium(*m_pcLEDMedium);
      AddComponent(c_block);
      m_vBoxes.push_back(&c_block);
      c_block.Update();
   }

   /****************************************/
   /****************************************/

   void CWallEntity::PositionBlocks() {

     CBlockEntity* pcBlock;
     Real fWallLenght = m_unNumberBoxes * m_cSize.GetY();
     Real fFirstBox = (-fWallLenght/2)+(m_cSize.GetY()/2);
     CRadians fAngleZ, fAngleY, fAngleX;
     m_pcPositionalEntity->GetOrientation().ToEulerAngles(fAngleZ,fAngleY,fAngleX);

     for(UInt32 i = 0; i < m_unNumberBoxes; ++i) {
       std::ostringstream id;

       id << this->GetId() << ".block_" << (i+1);
       pcBlock = new CBlockEntity(this,
                                  id.str().c_str(),
                                  m_pcPositionalEntity->GetPosition() + CVector3((fFirstBox)+(i*m_cSize.GetY())*Sin(-fAngleZ),
                                                                                 (fFirstBox)+(i*m_cSize.GetY())*Cos(-fAngleZ),
                                                                                 0),
                                  m_pcPositionalEntity->GetOrientation(),
                                  false,
                                  m_cSize,
                                  m_fMass);
       AddBlock(*pcBlock);
     }
   }

   /****************************************/
   /****************************************/


   REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CWallEntity);

   /****************************************/
   /****************************************/

}

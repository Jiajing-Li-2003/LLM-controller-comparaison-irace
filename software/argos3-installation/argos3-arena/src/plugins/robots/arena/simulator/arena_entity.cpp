/**
 * @file <argos3/plugins/robots/arena/simulator/arena_entity.cpp>
 *
 * @author David Garzon-Ramos - <dgarzonr@ulb.ac.be>
 */

#include "arena_entity.h"

namespace argos {


   /****************************************/
   /****************************************/

   CArenaEntity::CArenaEntity(const std::string& str_id,
                              const CVector3& c_position,
                              const CQuaternion& c_orientation,
                              const CVector3& c_size,
                              const std::string& str_led_medium,
                              UInt32 un_boxes,
                              UInt32 un_edges,
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
         m_unNumberEdges = un_edges;

         AddComponent(*m_pcPositionalEntity);

         PositionWalls();

         /* Update components */
         UpdateComponents();

      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize entity \"" << GetId() << "\".", ex);
      }
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::Init(TConfigurationNode& t_tree) {
      try {
         // TODO

           /*
             * Init parent
             */
           CComposableEntity::Init(t_tree);


         /* Update components */
         UpdateComponents();

      }
      catch(CARGoSException& ex) {
         THROW_ARGOSEXCEPTION_NESTED("Failed to initialize entity \"" << GetId() << "\".", ex);
      }
   }


   /****************************************/
   /****************************************/

   void CArenaEntity::Reset() {
      /* Reset all components */
      CComposableEntity::Reset();
      /* Update components */
      UpdateComponents();
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::Destroy() {

       CComposableEntity::Destroy();
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::AddWall(CWallEntity& c_wall) {
      AddComponent(c_wall);
      m_vWalls.push_back(&c_wall);
      c_wall.Update();
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::PositionWalls() {
     CWallEntity* pcWall;
     CRadians fAngle = (2 * CRadians::PI) / m_unNumberEdges;
     Real fRadious = InnerRadious();
     Real fWallLenght = m_unNumberBoxes * m_cSize.GetY();
     Real fFirstBox = (-fWallLenght/2)+(m_cSize.GetY()/2);
     for(UInt32 i = 1; i <= m_unNumberEdges; ++i) {
       std::ostringstream id;

       id << this->GetId() << ".wall_" << i;
       pcWall = new CWallEntity(this,
                                id.str().c_str(),
                                m_pcPositionalEntity->GetPosition() + CVector3(-fFirstBox,-fFirstBox,0)
                                                                    + CVector3((fRadious * Cos(fAngle * i))-(fFirstBox * Sin(-CRadians::PI + (fAngle * i))),
                                                                               (fRadious * Sin(fAngle * i))+(fFirstBox * Cos(-CRadians::PI + (fAngle * i))),
                                                                               0),
                                CQuaternion().FromEulerAngles(-CRadians::PI + (fAngle * i),CRadians::ZERO,CRadians::ZERO),
                                m_cSize,
                                "leds",
                                m_unNumberBoxes,
                                m_fGap,
                                m_fMass);
       AddWall(*pcWall);
     }
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::SetArenaColor(CColor vColor){
       CArenaEntity* pcArena = this;
       CWallEntity* pcWall;
       CBlockEntity* pcBlock;
       std::vector<CWallEntity*> pcWalls = pcArena->GetWalls();

       for (std::vector<CWallEntity*>::iterator itW = pcWalls.begin(); itW!=pcWalls.end(); ++itW){
           pcWall = m_vWalls.at(std::distance(pcWalls.begin(),itW));
           std::vector<CBlockEntity*> pcBlocks = pcWall->GetBlocks();
           for (std::vector<CBlockEntity*>::iterator itB = pcBlocks.begin(); itB!=pcBlocks.end(); ++itB){
               pcBlock = pcBlocks.at(std::distance(pcBlocks.begin(),itB));
               CLEDEquippedEntity& pcLED = pcBlock->GetLEDEquippedEntity();
               pcLED.SetAllLEDsColors(vColor);
               pcLED.Update();
           }
       }
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::SetWallColor(UInt32 unWallID, CColor vColor){
       CWallEntity* pcWall;
       CBlockEntity* pcBlock;
       pcWall = m_vWalls.at(unWallID-1);
       std::vector<CBlockEntity*> pcBlocks = pcWall->GetBlocks();
       for (std::vector<CBlockEntity*>::iterator it = pcBlocks.begin(); it!=pcBlocks.end(); ++it){
           pcBlock = pcBlocks.at(std::distance(pcBlocks.begin(),it));
           CLEDEquippedEntity& pcLED = pcBlock->GetLEDEquippedEntity();
           pcLED.SetAllLEDsColors(vColor);
           pcLED.Update();
       }
   }

   /****************************************/
   /****************************************/

   void CArenaEntity::SetBoxColor(UInt32 unBoxID, UInt32 unWallID, CColor vColor){

       CWallEntity* pcWall;
       pcWall = m_vWalls.at(unWallID-1);
       CBlockEntity* pcBlock;
       pcBlock = pcWall->GetBlocks().at(unBoxID-1);
       CLEDEquippedEntity& pcLED = pcBlock->GetLEDEquippedEntity();
       pcLED.SetAllLEDsColors(vColor);
       pcLED.Update();
   }

   /****************************************/
   /****************************************/

   bool CArenaEntity::IsEven(UInt32 unNumber) {
       bool even;
       if((unNumber%2)==0)
          even = true;
       else
          even = false;

       return even;
   }

   /****************************************/
   /****************************************/

   Real CArenaEntity::InnerRadious() {
       Real fInnerRadious = (m_cSize.GetY() * m_unNumberBoxes) / (2 * Tan(CRadians::PI / m_unNumberEdges));
       return fInnerRadious;
   }

   /****************************************/
   /****************************************/


   REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CArenaEntity);

   /****************************************/
   /****************************************/

}

/**
 * @file <argos3/plugins/simulator/visualizations/qt-opengl/models/qtopengl_box.h>
 *
 * @author Carlo Pinciroli - <ilpincy@gmail.com>
 */

#ifndef QTOPENGL_BLOCK_H
#define QTOPENGL_BLOCK_H

namespace argos {
   class CQTOpenGLBlock;
   class CBlockEntity;
}

#ifdef __APPLE__
#include <gl.h>
#else
#include <GL/gl.h>
#endif

namespace argos {

   class CQTOpenGLBlock {

   public:

      CQTOpenGLBlock();

      virtual ~CQTOpenGLBlock();

      virtual void DrawLEDs(CBlockEntity& c_entity);
      virtual void Draw(const CBlockEntity& c_entity);

   private:

      void MakeBody();
      void MakeLED();

   private:

      GLuint m_unBaseList;
      GLuint m_unBodyList;
      GLuint m_unLEDList;
      GLuint m_unVertices;

   };

}

#endif

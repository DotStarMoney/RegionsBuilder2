#ifndef SPRITEMACHINE_H
#define SPRITEMACHINE_H

#include <QVector>
#include <QImage>
#include "gldrawsurface.h"
#include <QGLShaderProgram>
#include <QPair>
#include "keybank.h"
#include "errlog.h"

#define COARSE_MEM_SCALE_START 5
#define COARSE_MEM_SCALE 20
#define MEM_SCALE_CONT 0.999

namespace SpriteMachine{

    typedef unsigned long int key;
    typedef unsigned int flag;

    const flag DRAW_FLAT = 0;
    const flag DRAW_TILED = 1;

    const float UNIFORM_DEPTH = 0;

    struct BoundingBox{
        BoundingBox(){}
        BoundingBox(float _x0, float _y0, float _x1, float _y1){
            x0 = _x0;
            y0 = _y0;
            x1 = _x1;
            y1 = _y1;
        }
        float x0;
        float y0;
        float x1;
        float y1;
    };

    struct VertexAttribArray{
        QVector3D p;
        QVector2D t;
        QVector2D s;
        QVector2D s_lo;
        QVector2D s_hi;
        float     a;
        QVector3D o;
        float     i;
        QVector4D h;
    };

    struct VertexAttribArray_Flat{
        QVector3D p;
        QVector2D t;
    };

    struct SpriteProperties{
        friend class Machine;
    public:
        SpriteProperties(){}
        SpriteProperties(flag flags,
                         BoundingBox location,
                         key atlas){
            this->flags = flags;
            this->location = location;
            this->atlas = atlas;
            this->sz = QVector2D(location.x1 - location.x0 + 1, location.y1 - location.y0 + 1);
        }
    private:
        flag        flags;
        BoundingBox location;
        QVector2D   sz;
        key         atlas;
        float       texCoords[4];
    };

    struct AtlasProperties{
        friend class Machine;
        AtlasProperties(){}
        AtlasProperties(QImage _atlas, float _invW, float _invH){
            atlas = _atlas;
            invW = _invW;
            invH = _invH;
        }

    private:
        QImage atlas;
        float  invW;
        float  invH;
    };

    struct SpriteSyncData{
        friend class Machine;
        SpriteSyncData(){
            size = 0;
            capacity = COARSE_MEM_SCALE_START;
            data   = (VertexAttribArray*) malloc((sizeof(VertexAttribArray)*4) * capacity);
            i_data = (GLushort*)          malloc((sizeof(GLushort)*4) * capacity);
        }
        void reset(int c){
            size = 0;
            capacity = c;
            data   = (VertexAttribArray*) realloc((void*) data,   (sizeof(VertexAttribArray)*4) * capacity);
            i_data = (GLushort*)          realloc((void*) i_data, (sizeof(GLushort)*4) * capacity);
        }

    private:
        int                capacity;
        int                size;
        VertexAttribArray* data;
        GLushort*          i_data;
    };

    class Machine : public ErrLog::Log
    {
    public:
        Machine(GLDrawSurface* parent);
        ~Machine();

        void transferPickTexture();
        float pick(int index);

        void clearAtlasCache();
        void clearSpriteCache();
        void removeSprite(key spriteNum);
        void removeAtlas(key atlasNum);

        key loadAtlas(const char* filename);
        key create(key atlasNum,
                   BoundingBox loc);
        int getWidth(key num);
        int getHeight(key num);
        void setDrawWidth(key num, float w);
        void setDrawHeight(key num, float h);
        void setAngle(key num, float a);
        float getAngle(key num);

        void enableTile(key num);
        void disableTile(key num);

        void draw(key num,
                  BoundingBox loc,
                  float angle = 0.0, float id = 32.0,
                  QVector4D hilight = QVector4D(0.0,0.0,0.0,0.0));
        void setMemoryBeta(float b);

        void sync(bool pick = false);
    private:
        QGLShaderProgram flat_shader;
        QGLShaderProgram copy_shader;
        KeyBank<key>     keys;
        int              attrLoc[11];
        bool             hasPickingTexture;
        GLuint           pickTexture;
        bool             hasTransferedPickTexture;
        float*           pickTextureData;
        int              pickWidth;
        int              pickHeight;

        GLDrawSurface* parent;

        QVector<AtlasProperties>  atlasList;
        QVector<SpriteProperties> spriteList;
        QVector<int>              avgSpritePerSync;
        QVector<int>              spriteTotal;
        float                     linDec;

        QVector<SpriteSyncData>   spriteToSync;
    };

}

#endif // SPRITEMACHINE_H

#ifndef TEXTMACHINE_H
#define TEXTMACHINE_H

#include "../gldrawsurface.h"
#include "fontface.h"
#include "../errlog.h"
#include "../udynamicarray.h"
#include <QGLShaderProgram>
#include <QVector3D>
#include <QVector4D>
#include <QHash>
#include <QString>

namespace TextMachine{

    struct VertexAttrib{
        QVector3D p;
        QVector2D t;
    };

    struct InstanceAttrib{
        GLfloat   channel;
        GLfloat   unused0;
        GLfloat   unused1;
        GLfloat   unused2;
        QVector4D color;
    };

    struct VertexAttribArrays{
        friend class Machine;
        VertexAttribArrays(){ triangles = 0; }
    private:
        uDynamicArray<VertexAttrib>   verts;
        uDynamicArray<InstanceAttrib> insts;
        uDynamicArray<GLushort>       indices;
        int                           triangles;
    };

    class Machine : public ErrLog::Log
    {
    public:
        Machine(GLDrawSurface* _parent);
        ~Machine();
        void sync();
        void drawText(float x, float y,
                      QString text, QString font,
                      int color = 0xffffffff, float scale = 1.0);
        QString loadFont(QString filename);
        void unloadFont(QString name);
        void flush();
    private:
        QGLShaderProgram text_shader;
        int attrLoc[4];
        GLDrawSurface* parent;

        static QHash<QString, Fontface*> fontStore;
        QHash<QString, VertexAttribArrays*> charsToSync;
        bool isDeprecated;

    };


}

#endif // TEXTMACHINE_H

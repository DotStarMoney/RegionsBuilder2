#ifndef PRIMITIVEMACHINE_H
#define PRIMITIVEMACHINE_H

#include "gldrawsurface.h"
#include <QGLShaderProgram>
#include <QVector3D>
#include <QVector4D>

#define COARSE_MEM_SCALE_START_V 16
#define COARSE_MEM_SCALE_START_I 72
#define COARSE_MEM_SCALE 20
#define MEM_SCALE_CONT 0.999

#define PI 3.14159265359

namespace PrimitiveMachine{

    const float UNIFORM_DEPTH = 0;
    const QVector4D TRANS_COLOR = QVector4D(0.0, 0.0, 0.0, 0.0);

    struct VertexAttribArray{
        friend class Machine;
        VertexAttribArray(){}
        VertexAttribArray(QVector3D _p, QVector4D _c){
            p = _p;
            c = _c;
        }
    private:
        QVector3D p;
        QVector4D c;
    };

    struct LineSyncData{
        friend class Machine;
        LineSyncData(){
            v_size = 0;
            i_size = 0;
            v_capacity = COARSE_MEM_SCALE_START_V;
            i_capacity = COARSE_MEM_SCALE_START_I;
            v_data = (VertexAttribArray*) malloc(sizeof(VertexAttribArray) * v_capacity);
            i_data = (GLushort*)          malloc(sizeof(GLushort) * i_capacity);
        }
        ~LineSyncData(){
            free((void*) v_data);
            free((void*) i_data);
        }

        void reset(int v_s, int i_s){
            v_size = 0;
            i_size = 0;
            v_capacity = v_s;
            i_capacity = i_s;
            v_data = (VertexAttribArray*) realloc((void*) v_data, sizeof(VertexAttribArray) * v_capacity);
            i_data = (GLushort*)          realloc((void*) i_data, sizeof(GLushort) * i_capacity);
        }
        void addVertex(VertexAttribArray v){
            v_size++;
            if(v_size > v_capacity){
                v_capacity += COARSE_MEM_SCALE;
                v_data = (VertexAttribArray*) realloc((void*) v_data, sizeof(VertexAttribArray) * v_capacity);
            }
            v_data[v_size-1] = v;
        }
        void addVertex(QVector2D _p, QVector4D _c){
            addVertex(VertexAttribArray(QVector3D(_p.x(), _p.y(), UNIFORM_DEPTH), _c));
        }
        void addIndex(GLushort ind){
            i_size++;
            if(i_size > i_capacity){
                i_capacity += COARSE_MEM_SCALE;
                i_data = (GLushort*) realloc((void*) i_data, sizeof(GLushort) * i_capacity);
            }
            i_data[i_size-1] = ind;
        }

    private:
        int                v_capacity;
        int                i_capacity;
        int                v_size;
        int                i_size;
        VertexAttribArray* v_data;
        GLushort*          i_data;
    };

    class Machine
    {
    public:
        Machine(GLDrawSurface* _parent);
        void sync();
        void drawPolyline(int npoints, QVector2D* pts, QVector4D color,  float lineW = 1.0, float alias_edge = 2.0);
        void drawLine(QVector2D a, QVector2D b, QVector4D color, float lineW = 1.0, float alias_edge = 2.0);
        void drawCircle(QVector2D pos, float radius, QVector4D color, float lineW = 1.0, float alias_edge = 2.0);
        void drawDottedBox(QVector2D a, QVector2D b, QVector4D color,  float lineW = 1.5, float alias_edge = 2.0);
        void drawRotatedBox(QVector2D pos, float radius, float angle, QVector4D color, float lineW = 1.0, float alias_edge = 2.0);
        void setMemoryBeta(float b);
    private:
        QGLShaderProgram line_shader;
       // static bool             line_shader_init;

        int attrLoc[2];
        GLDrawSurface* parent;

        LineSyncData vbo_data;
        int vertTotal;
        int avgVertPerSync;

        int indTotal;
        int avgIndPerSync;

        float linDec;
    };

}
#endif // PRIMITIVEMACHINE_H

#include "primitivemachine.h"
#include <QtCore/QtMath>

#define A_POSITION 0
#define A_COLOR    1

#define TARGET_SEG_LENGTH(x) (2.2 + 0.06 * (x))


using namespace PrimitiveMachine;

//QGLShaderProgram Machine::line_shader;
//bool Machine::line_shader_init = false;

Machine::Machine(GLDrawSurface* _parent){
    parent = _parent;
    //if(!line_shader_init){
    //    line_shader_init = true;
        if(!line_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/lineart.vert")) parent->close();
        if(!line_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/lineart.frag")) parent->close();
        if(!line_shader.link()) parent->close();
        attrLoc[0] = line_shader.attributeLocation("a_position");
        attrLoc[1] = line_shader.attributeLocation("a_color");
        line_shader.enableAttributeArray(attrLoc[A_POSITION]);
        line_shader.enableAttributeArray(attrLoc[A_COLOR]);
    //}
    avgIndPerSync  = COARSE_MEM_SCALE_START_I;
    avgVertPerSync = COARSE_MEM_SCALE_START_V;
    vertTotal = 0;
    indTotal = 0;
    linDec = MEM_SCALE_CONT;
}

void Machine::setMemoryBeta(float b){
    linDec = b;
}

void Machine::drawPolyline(int npoints, QVector2D *pts, QVector4D color, float lineW, float alias_edge){
    float coreExtend;
    float edgeExtend;
    float ex[4];
    float closeAngle;
    int i;
    int firstVertex;
    int firstIndex;
    int curIG;
    int nxtIG;
    int ibound;
    int wbound;
    bool isCyclic;
    QVector2D aD;
    QVector2D bD;
    QVector2D vExt;
    QVector2D vExt_p;
    QVector2D vBase;
    QVector2D vFirstSub;

    coreExtend = (lineW - (alias_edge*0.5)) * 0.5;
    edgeExtend = coreExtend + alias_edge;

    isCyclic = (pts[0] == pts[npoints - 1]);

    firstVertex = vbo_data.v_size;
    firstIndex  = vbo_data.i_size;

    if(!isCyclic){
        vExt = pts[1] - pts[0];
        vExt.normalize();
        vExt_p = QVector2D(vExt.y(), -vExt.x());

        vBase = pts[0] - vExt * edgeExtend;
        vbo_data.addVertex(vBase - vExt_p * edgeExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase - vExt_p * coreExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase + vExt_p * coreExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase + vExt_p * edgeExtend, TRANS_COLOR);

        vertTotal += 4;
        vBase = pts[0] - vExt * coreExtend;
        ex[0] = edgeExtend;
        ex[1] = coreExtend;
        ex[2] = coreExtend;
        ex[3] = edgeExtend;
    }
    else{
        vBase = pts[0];
        aD = (pts[npoints - 1] - pts[npoints - 2]);
        bD = (pts[0] - pts[1]);
        aD.normalize();
        bD.normalize();
        vExt_p = aD + bD;
        vExt_p.normalize();
        vExt = pts[npoints - 1] - pts[npoints - 2];
        vExt.normalize();
        vExt = QVector2D(vExt.y(), -vExt.x());
        if((vExt_p.x() == 0.0) && (vExt_p.y() == 0.0)){
            vExt_p = vExt;
            ex[0] = edgeExtend;
            ex[1] = coreExtend;
            ex[2] = coreExtend;
            ex[3] = edgeExtend;
        }
        else{
            if(QVector2D::dotProduct(vExt_p, vExt) < 0){
                vExt_p = -vExt_p;

                bD = QVector2D(bD.y(), -bD.x());
                closeAngle = QVector2D::dotProduct(-vExt_p, bD);
                ex[0] = edgeExtend / closeAngle;
                ex[1] = coreExtend / closeAngle;
                closeAngle = qSin(1.5708 - acos(closeAngle));
                ex[2] = coreExtend / closeAngle;
                ex[3] = edgeExtend / closeAngle;
            }
            else{
                aD = QVector2D(aD.y(), -aD.x());
                closeAngle = QVector2D::dotProduct(vExt_p, aD);
                ex[3] = edgeExtend / closeAngle;
                ex[2] = coreExtend / closeAngle;
                closeAngle = qSin(1.5708 - acos(closeAngle));
                ex[1] = coreExtend / closeAngle;
                ex[0] = edgeExtend / closeAngle;
            }
        }



    }

    vbo_data.addVertex(vBase - vExt_p * ex[0], TRANS_COLOR);
    vbo_data.addVertex(vBase - vExt_p * ex[1], color);
    vbo_data.addVertex(vBase + vExt_p * ex[2], color);
    vbo_data.addVertex(vBase + vExt_p * ex[3], TRANS_COLOR);

    vertTotal += 4;

    for(i = 1; i < npoints - 1; i++){
        vBase = pts[i];
        vFirstSub = pts[i] - pts[i - 1];
        vFirstSub.normalize();
        bD = pts[i] - pts[i + 1];
        bD.normalize();
        vExt_p = vFirstSub + bD;
        vExt_p.normalize();
        vExt = vFirstSub;
        vExt = QVector2D(vExt.y(), -vExt.x());
        if((vExt_p.x() == 0.0) && (vExt_p.y() == 0.0)){
            vExt_p = vExt;
            ex[0] = edgeExtend;
            ex[1] = coreExtend;
            ex[2] = coreExtend;
            ex[3] = edgeExtend;
        }
        else{
            if(QVector2D::dotProduct(vExt_p, vExt) < 0){
                vExt_p = -vExt_p;

                bD = QVector2D(bD.y(), -bD.x());
                closeAngle = QVector2D::dotProduct(-vExt_p, bD);
                ex[0] = edgeExtend / closeAngle;
                ex[1] = coreExtend / closeAngle;
                closeAngle = qSin(1.5708 - acos(closeAngle));
                ex[2] = coreExtend / closeAngle;
                ex[3] = edgeExtend / closeAngle;
            }
            else{

                aD = QVector2D(vFirstSub.y(), -vFirstSub.x());
                closeAngle = QVector2D::dotProduct(vExt_p, aD);
                ex[3] = edgeExtend / closeAngle;
                ex[2] = coreExtend / closeAngle;
                closeAngle = qSin(1.5708 - acos(closeAngle));
                ex[1] = coreExtend / closeAngle;
                ex[0] = edgeExtend / closeAngle;
            }
        }
        vbo_data.addVertex(vBase - vExt_p * ex[0], TRANS_COLOR);
        vbo_data.addVertex(vBase - vExt_p * ex[1], color);
        vbo_data.addVertex(vBase + vExt_p * ex[2], color);
        vbo_data.addVertex(vBase + vExt_p * ex[3], TRANS_COLOR);

        vertTotal += 4;
    }

    if(!isCyclic){
        vExt = pts[npoints - 1] - pts[npoints - 2];
        vExt.normalize();
        vExt_p = QVector2D(vExt.y(), -vExt.x());

        vBase = pts[npoints - 1] + vExt * coreExtend;
        vbo_data.addVertex(vBase - vExt_p * edgeExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase - vExt_p * coreExtend, color);
        vbo_data.addVertex(vBase + vExt_p * coreExtend, color);
        vbo_data.addVertex(vBase + vExt_p * edgeExtend, TRANS_COLOR);

        vBase = pts[npoints - 1] + vExt * edgeExtend;
        vbo_data.addVertex(vBase - vExt_p * edgeExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase - vExt_p * coreExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase + vExt_p * coreExtend, TRANS_COLOR);
        vbo_data.addVertex(vBase + vExt_p * edgeExtend, TRANS_COLOR);

        vertTotal += 8;

        ibound = npoints + 1;
        wbound = npoints + 2;
    }
    else{
        ibound = npoints - 1;
        wbound = npoints - 1;
    }


    nxtIG = firstVertex;
    for(i = 0; i < ibound; i++){
        curIG = nxtIG;
        nxtIG = firstVertex + ((i + 1) % (wbound)) * 4;
        vbo_data.addIndex(curIG);
        vbo_data.addIndex(nxtIG);
        vbo_data.addIndex(curIG + 1);

        vbo_data.addIndex(curIG + 1);
        vbo_data.addIndex(nxtIG);
        vbo_data.addIndex(nxtIG + 1);

        vbo_data.addIndex(curIG + 1);
        vbo_data.addIndex(nxtIG + 1);
        vbo_data.addIndex(curIG + 2);

        vbo_data.addIndex(curIG + 2);
        vbo_data.addIndex(nxtIG + 1);
        vbo_data.addIndex(nxtIG + 2);

        vbo_data.addIndex(curIG + 2);
        vbo_data.addIndex(nxtIG + 2);
        vbo_data.addIndex(curIG + 3);

        vbo_data.addIndex(curIG + 3);
        vbo_data.addIndex(nxtIG + 2);
        vbo_data.addIndex(nxtIG + 3);

        indTotal += 18;
    }



}

void Machine::drawCircle(QVector2D pos, float radius, QVector4D color, float lineW, float alias_edge){
    float upperRad;
    float circ;
    int   circSize;
    float stepSize;
    float angle;

    QVector2D* dotPos;

    int i;

    upperRad = radius + 0.5 * lineW;
    circ = upperRad * 2.0 * 3.14159;

    circSize = ceil(circ / TARGET_SEG_LENGTH(radius));
    stepSize = circ / (float) circSize;
    dotPos = new QVector2D[circSize + 1];

    for(i = 0; i < circSize; i++){
        angle = ((float) i / circSize) * 6.28319;
        dotPos[i] = pos + QVector2D(cos(angle) * radius, qSin(angle) * radius);
    }

    dotPos[circSize] = dotPos[0];

    drawPolyline(circSize+1, dotPos, color, lineW, alias_edge);

}

void Machine::drawLine(QVector2D a, QVector2D b, QVector4D color, float lineW, float alias_edge){
    QVector2D pts[] = {a, b};
    drawPolyline(2, pts, color, lineW, alias_edge);
}


#define DB_CORNER_LENGTH 5.0
#define DB_THICKNESS lineW

void Machine::drawDottedBox(QVector2D a, QVector2D b, QVector4D color, float lineW, float alias_edge){
    QVector2D tl;
    QVector2D br;
    float     width;
    float     height;
    QVector2D corner[3];
    QVector2D cornerPiece_width;
    QVector2D cornerPiece_height;
    QVector2D curPoint;

    tl = QVector2D(qMin(a.x(), b.x()), qMin(a.y(), b.y()));
    br = QVector2D(qMax(a.x(), b.x()), qMax(a.y(), b.y()));
    width  = br.x() - tl.x();
    height = br.y() - tl.y();

    cornerPiece_width  = QVector2D(qMin(width, (float) DB_CORNER_LENGTH), 0.0);
    cornerPiece_height = QVector2D(0.0, qMin(height, (float) DB_CORNER_LENGTH));

    if((a.x() != b.x()) && (a.y() != b.y())){
        corner[1] = tl;
        corner[0] = corner[1] + cornerPiece_height;
        corner[2] = corner[1] + cornerPiece_width;
        drawPolyline(3, corner, color, DB_THICKNESS);

        corner[1] = QVector2D(br.x(), tl.y());
        corner[0] = corner[1] - cornerPiece_width;
        corner[2] = corner[1] + cornerPiece_height;
        drawPolyline(3, corner, color, DB_THICKNESS);

        corner[1] = br;
        corner[0] = corner[1] - cornerPiece_height;
        corner[2] = corner[1] - cornerPiece_width;
        drawPolyline(3, corner, color, DB_THICKNESS);

        corner[1] = QVector2D(tl.x(), br.y());
        corner[0] = corner[1] + cornerPiece_width;
        corner[2] = corner[1] - cornerPiece_height;
        drawPolyline(3, corner, color, DB_THICKNESS);
    }
    else if((a.x() != b.x()) && (a.y() == b.y())){
        drawLine(tl, tl + cornerPiece_width, color, DB_THICKNESS);
        drawLine(br, br - cornerPiece_width, color, DB_THICKNESS);
    }
    else if((a.x() == b.x()) && (a.y() != b.y())){
        drawLine(tl, tl + cornerPiece_height, color, DB_THICKNESS);
        drawLine(br, br - cornerPiece_height, color, DB_THICKNESS);
    }
    else{
        return;
    }

    if(a.x() != b.x()){
        curPoint = tl + cornerPiece_width * 3;
        while(QVector2D(curPoint + cornerPiece_width * 2).x() <= br.x()){
            drawLine(curPoint,                        curPoint + cornerPiece_width,                        color, DB_THICKNESS);
            drawLine(QVector2D(curPoint.x(), br.y()), QVector2D(curPoint.x(), br.y()) + cornerPiece_width, color, DB_THICKNESS);
            curPoint = curPoint + cornerPiece_width * 3;
        }
    }
    if(a.y() != b.y()){
        curPoint = tl + cornerPiece_height * 3;;
        while(QVector2D(curPoint + cornerPiece_height * 2).y() <= br.y()){
            drawLine(curPoint,                        curPoint + cornerPiece_height,                        color, DB_THICKNESS);
            drawLine(QVector2D(br.x(), curPoint.y()), QVector2D(br.x(), curPoint.y()) + cornerPiece_height, color, DB_THICKNESS);
            curPoint = curPoint + cornerPiece_height * 3;
        }
    }
}

void Machine::drawRotatedBox(QVector2D pos, float radius, float angle, QVector4D color, float lineW, float alias_edge){
    QVector2D pts[5];
    pts[0] = pos + QVector2D(cos(angle + PI/4), qSin(angle + PI/4)) * radius;
    pts[1] = pos + QVector2D(cos(angle + 3*PI/4), qSin(angle + 3*PI/4)) * radius;
    pts[2] = pos + QVector2D(cos(angle - 3*PI/4), qSin(angle - 3*PI/4)) * radius;
    pts[3] = pos + QVector2D(cos(angle - PI/4), qSin(angle - PI/4)) * radius;
    pts[4] = pts[0];
    drawPolyline(5, pts, color, lineW, alias_edge);
}


void Machine::sync(){
#define ADD_ATTRIBUTE_LOCATION(ASIZE, ALOC, OFFS)                                                                           \
    (                                                                                                                       \
        (parent->vertexAttribPointer(ALOC, ASIZE, GL_FLOAT, GL_FALSE, sizeof(VertexAttribArray), (const void*) offset)),    \
        (offset += OFFS),                                                                                                   \
        (void)0                                                                                                             \
    )

    quintptr offset;
    GLuint vbo[2];
    bool bindOnce;

    offset = 0;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    line_shader.bind();
    line_shader.setUniformValue("mvp_matrix", parent->getOrtho());

    parent->genBuffers(2, vbo);
    bindOnce = false;

    parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    parent->bufferData(GL_ARRAY_BUFFER, vbo_data.v_size * sizeof(VertexAttribArray), vbo_data.v_data, GL_DYNAMIC_DRAW);
    parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);

    if(!bindOnce){
        ADD_ATTRIBUTE_LOCATION(3, attrLoc[A_POSITION], sizeof(QVector3D));
        ADD_ATTRIBUTE_LOCATION(4, attrLoc[A_COLOR], sizeof(QVector4D));
        bindOnce = true;
    }

    parent->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    parent->bufferData(GL_ELEMENT_ARRAY_BUFFER, vbo_data.i_size * sizeof(GLushort), vbo_data.i_data, GL_DYNAMIC_DRAW);
    parent->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

    parent->drawElements(GL_TRIANGLES, vbo_data.i_size, GL_UNSIGNED_SHORT, 0);

    if(vertTotal >= avgVertPerSync){
        vertTotal = avgVertPerSync;
    }
    else{
        avgVertPerSync = ((float) avgVertPerSync) * linDec + ((float) vertTotal) * (1 - linDec);
    }
    if(indTotal >= avgIndPerSync){
        indTotal = avgIndPerSync;
    }
    else{
        avgIndPerSync = ((float) avgIndPerSync) * linDec + ((float) indTotal) * (1 - linDec);
    }
    vertTotal = 0;
    indTotal = 0;

    vbo_data.reset(avgVertPerSync, avgIndPerSync);
    parent->deleteBuffers(2, vbo);
}

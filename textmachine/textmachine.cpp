#include "textmachine.h"
#include <QtOpenGL>
#include <QDebug>

#define A_POSITION 0
#define A_TEXCOORD 1
#define A_COLOR    2
#define A_CHANNEL  3

#define GL_MAJOR_THRESH 4
#define GL_MINOR_THRESH 2

using namespace TextMachine;

QHash<QString, Fontface*> Machine::fontStore;

Machine::Machine(GLDrawSurface* _parent){
    int     glmajor;
    int     glminor;
    QString deprecated;

    this->parent = _parent;

    // TODO, BETTER TEXBUFFER SUPPORT
    // right now we need a way to verify graphical features,
    // will come to this later

    parent->getIntegerv(GL_MAJOR_VERSION, &glmajor);
    parent->getIntegerv(GL_MINOR_VERSION, &glminor);

    isDeprecated = true;//false;
    if(glmajor < GL_MAJOR_THRESH){
        isDeprecated = true;
    }
    else if(glminor < GL_MINOR_THRESH){
        isDeprecated = true;
    }

    if(isDeprecated)
        deprecated = "_deprecated";
    else
        deprecated = "";

    if(!text_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/text" + deprecated + ".vert")){
        error("could not load vertex shader");
        parent->close();
    }
    if(!text_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/text" + deprecated + ".frag")){
        error("could not load fragment shader");
        parent->close();
    }
    if(!text_shader.link()){
        error("could not link shader");
        parent->close();
    }
    attrLoc[A_POSITION] = text_shader.attributeLocation("a_position");
    attrLoc[A_TEXCOORD] = text_shader.attributeLocation("a_texcoord");
    text_shader.enableAttributeArray(attrLoc[A_POSITION]);
    text_shader.enableAttributeArray(attrLoc[A_TEXCOORD]);
    if(isDeprecated){
        attrLoc[A_COLOR] = text_shader.attributeLocation("a_color");
        attrLoc[A_CHANNEL] = text_shader.attributeLocation("a_channel");
        text_shader.enableAttributeArray(attrLoc[A_COLOR]);
        text_shader.enableAttributeArray(attrLoc[A_CHANNEL]);
    }
}

Machine::~Machine(){
    flush();
}

void Machine::drawText(float x, float y,
               QString text, QString font,
               int color, float scale){

    QVector2D* p;
    QVector2D* t;
    QVector2D  pos;
    int*       chnl;
    int        iOff;
    int        vOff;
    int        sOff;
    float      width, height;
    int        i, i4, q;
    VertexAttribArrays* syncData;

    Fontface* curFont;
    if(!fontStore.contains(font)){
        warning("no such font exists");
        return;
    }
    curFont = fontStore.value(font);

    if(!charsToSync.contains(font)){
        charsToSync.insert(font, new VertexAttribArrays());
        syncData = charsToSync.value(font);
    }
    else{
        syncData = charsToSync.value(font);
    }

    iOff = syncData->indices.size();
    vOff = syncData->verts.size();
    sOff = syncData->insts.size();

    syncData->indices.reserve(6 * text.length());
    syncData->verts.reserve(4 * text.length());
    syncData->insts.reserve((isDeprecated ? 4 : 1) * text.length());

    curFont->computeTextGeometry(text, scale, p, t, chnl, width, height);
    pos = QVector2D(x, y);
    for(i = 0; i < text.length(); i++){
        i4 = i * 4;
        syncData->verts.data()[vOff + 0].p = QVector3D(p[i4 + 0] + pos);
        syncData->verts.data()[vOff + 0].t = t[i4 + 0];
        syncData->verts.data()[vOff + 1].p = QVector3D(p[i4 + 1] + pos);
        syncData->verts.data()[vOff + 1].t = t[i4 + 1];
        syncData->verts.data()[vOff + 2].p = QVector3D(p[i4 + 2] + pos);
        syncData->verts.data()[vOff + 2].t = t[i4 + 2];
        syncData->verts.data()[vOff + 3].p = QVector3D(p[i4 + 3] + pos);
        syncData->verts.data()[vOff + 3].t = t[i4 + 3];

        syncData->insts.data()[sOff + 0].channel = chnl[i];
        syncData->insts.data()[sOff + 0].color.setW(((float)((color >> 24) & 0xFF)) / 255.0);
        syncData->insts.data()[sOff + 0].color.setX(((float)((color >> 16) & 0xFF)) / 255.0);
        syncData->insts.data()[sOff + 0].color.setY(((float)((color >>  8) & 0xFF)) / 255.0);
        syncData->insts.data()[sOff + 0].color.setZ(((float)((color >>  0) & 0xFF)) / 255.0);

        if(isDeprecated){
            for(q = 1; q < 4; q++){
                syncData->insts.data()[sOff + q].channel = syncData->insts.data()[sOff + 0].channel;
                syncData->insts.data()[sOff + q].color   = syncData->insts.data()[sOff + 0].color;
            }
        }

        syncData->indices.data()[iOff + 0] = vOff + 0;
        syncData->indices.data()[iOff + 1] = vOff + 3;
        syncData->indices.data()[iOff + 2] = vOff + 2;
        syncData->indices.data()[iOff + 3] = vOff + 1;
        syncData->indices.data()[iOff + 4] = vOff + 0;
        syncData->indices.data()[iOff + 5] = vOff + 2;

        sOff += (isDeprecated ? 4 : 1);
        vOff += 4;
        iOff += 6;
        syncData->triangles += 2;
    }
}

void Machine::sync(){
    #define ADD_ATTRIBUTE_LOCATION(ALOC, ASIZE, DATAT, ASTRIDE, OFFS)                                                                           \
    (                                                                                                                       \
        (parent->vertexAttribPointer(ALOC, ASIZE, DATAT, GL_FALSE, sizeof(ASTRIDE), (const void*) offset)),    \
        (offset += sizeof(OFFS)),                                                                                                   \
        (void)0                                                                                                             \
    )

    #define RESET_OFFSET() offset = 0
    GLuint vbo[3];
    GLuint tbo;
    QHash<QString, VertexAttribArrays*>::iterator curItem;
    VertexAttribArrays* d;
    quintptr offset;
    QImage curTex;
    Fontface* curFont;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    text_shader.bind();
    text_shader.setUniformValue("mvp_matrix", parent->getOrtho());
    text_shader.setUniformValue("texture", 0);
    if(!isDeprecated) text_shader.setUniformValue("instance_tbo", 1);

    parent->genBuffers(3, vbo);

    parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    RESET_OFFSET();
    ADD_ATTRIBUTE_LOCATION(attrLoc[A_POSITION], 3, GL_FLOAT, VertexAttrib, QVector3D);
    ADD_ATTRIBUTE_LOCATION(attrLoc[A_TEXCOORD], 2, GL_FLOAT, VertexAttrib, QVector2D);

    if(isDeprecated){
        parent->bindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        RESET_OFFSET();
        ADD_ATTRIBUTE_LOCATION(attrLoc[A_CHANNEL], 4, GL_FLOAT, InstanceAttrib, QVector4D);
        ADD_ATTRIBUTE_LOCATION(attrLoc[A_COLOR],   4, GL_FLOAT, InstanceAttrib, QVector2D);
    }
    else{
        parent->bindBuffer(GL_TEXTURE_BUFFER, vbo[1]);
        parent->genTextures(1, &tbo);

        parent->activeTexture(GL_TEXTURE1);
        parent->_bindTexture(GL_TEXTURE_BUFFER, tbo);

        parent->texBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, vbo[1]);
    }

    for(curItem = charsToSync.begin(); curItem != charsToSync.end(); curItem++){
        d = *curItem;
        curFont = fontStore.value(curItem.key());
        curTex = curFont->getPage();

        parent->activeTexture(GL_TEXTURE0);
        parent->bindTexture(curTex);

        parent->bindBuffer(isDeprecated ? GL_ARRAY_BUFFER : GL_TEXTURE_BUFFER, vbo[1]);
        parent->bufferData(isDeprecated ? GL_ARRAY_BUFFER : GL_TEXTURE_BUFFER, d->insts.size() * sizeof(InstanceAttrib), d->insts.data(), GL_DYNAMIC_DRAW);

        parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        parent->bufferData(GL_ARRAY_BUFFER, d->verts.size() * sizeof(VertexAttrib), d->verts.data(), GL_DYNAMIC_DRAW);

        parent->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
        parent->bufferData(GL_ELEMENT_ARRAY_BUFFER, d->indices.size() * sizeof(GLushort), d->indices.data(), GL_DYNAMIC_DRAW);

        parent->drawElements(GL_TRIANGLES, d->triangles * 3, GL_UNSIGNED_SHORT, 0);

        d->indices.clear();
        d->verts.clear();
        d->insts.clear();
        d->triangles = 0;
    }
    parent->deleteBuffers(3, vbo);
    parent->deleteTexture(tbo);
}


QString Machine::loadFont(QString filename){
    Fontface* font;
    font = new Fontface(filename);
    if(font->status() == ErrLog::STATUS_OK){
        fontStore.insert(font->getFace(), font);
        return font->getFace();
    }
    else{
        warning("unable to load font");
        delete font;
        return "";
    }
}

void Machine::unloadFont(QString name){
    if(fontStore.contains(name)){
        delete fontStore.value(name);
        fontStore.remove(name);
    }
    else{
        warning("fontface does not exist");
    }
}

void Machine::flush(){
    QHash<QString, Fontface*>::iterator i;
    for(i = fontStore.begin(); i != fontStore.end(); i++){
        delete (*i);
    }
}

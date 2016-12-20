#include "spritemachine.h"
#include <QDebug>

#define A_POSITION 0
#define A_TEXCOORD 1
#define A_SCALE    2
#define A_SCALE_LO 3
#define A_SCALE_HI 4
#define A_ANGLE    5
#define A_ORIGIN   6
#define A_ID       7
#define A_HILIGHT  8

#define A_POSITION_FLAT 9
#define A_TEXCOORD_FLAT 10

using namespace SpriteMachine;

Machine::Machine(GLDrawSurface* parent)
{
    this->parent = parent;
    linDec = (float) MEM_SCALE_CONT;
    hasPickingTexture        = false;
    hasTransferedPickTexture = false;
    pickTextureData = NULL;

    if(!flat_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/sprite.vert")) parent->close();
    if(!flat_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/sprite.frag")) parent->close();
    if(!flat_shader.link()) parent->close();
    attrLoc[A_POSITION] = flat_shader.attributeLocation("a_position");
    attrLoc[A_TEXCOORD] = flat_shader.attributeLocation("a_texcoord");
    attrLoc[A_SCALE]    = flat_shader.attributeLocation("a_scale");
    attrLoc[A_SCALE_LO] = flat_shader.attributeLocation("a_scale_lo");
    attrLoc[A_SCALE_HI] = flat_shader.attributeLocation("a_scale_hi");
    attrLoc[A_ANGLE]    = flat_shader.attributeLocation("a_angle");
    attrLoc[A_ORIGIN]   = flat_shader.attributeLocation("a_origin");
    attrLoc[A_ID]       = flat_shader.attributeLocation("a_id");
    attrLoc[A_HILIGHT]  = flat_shader.attributeLocation("a_hilight");
    flat_shader.enableAttributeArray(attrLoc[A_POSITION]);
    flat_shader.enableAttributeArray(attrLoc[A_TEXCOORD]);
    flat_shader.enableAttributeArray(attrLoc[A_SCALE]);
    flat_shader.enableAttributeArray(attrLoc[A_SCALE_LO]);
    flat_shader.enableAttributeArray(attrLoc[A_SCALE_HI]);
    flat_shader.enableAttributeArray(attrLoc[A_ANGLE]);
    flat_shader.enableAttributeArray(attrLoc[A_ORIGIN]);
    flat_shader.enableAttributeArray(attrLoc[A_ID]);
    flat_shader.enableAttributeArray(attrLoc[A_HILIGHT]);

    if(!copy_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/flat.vert")) parent->close();
    if(!copy_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/flat.frag")) parent->close();
    if(!copy_shader.link()) parent->close();
    attrLoc[A_POSITION_FLAT] = copy_shader.attributeLocation("a_position");
    attrLoc[A_TEXCOORD_FLAT] = copy_shader.attributeLocation("a_texcoord");
    copy_shader.enableAttributeArray(attrLoc[A_POSITION_FLAT]);
    copy_shader.enableAttributeArray(attrLoc[A_TEXCOORD_FLAT]);
}

Machine::~Machine(){
    int i;
    for(i = 0; i < spriteToSync.size(); i++){
        free((void*) spriteToSync[i].data);
        free((void*) spriteToSync[i].i_data);
    }
    if(hasPickingTexture) parent->deleteTexture(pickTexture);
    if(pickTextureData != NULL) free(pickTextureData);
}

void clearAtlasCache();
void clearSpriteCache();
void removeSprite(key spriteNum);
void removeAtlas(key atlasNum);

void Machine::transferPickTexture(){
    if(hasPickingTexture){
        if(pickTextureData != NULL) free(pickTextureData);
        pickTextureData = (float*) calloc(pickWidth*pickHeight, sizeof(float));

        parent->_bindTexture(GL_TEXTURE_2D, pickTexture);
        parent->getTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pickTextureData);

        hasTransferedPickTexture = true;
    }
    else{
        warning("pick texture already transferred");
    }
}

float Machine::pick(int index){
    if(hasTransferedPickTexture){
        return pickTextureData[index];
    }
    else{
        warning("pick texture has not been transferred from hardware");
        return 0.0;
    }
}

key Machine::loadAtlas(const char *filename){
    int    currentIndex;
    QImage atlasImage;

    currentIndex = atlasList.size();

    atlasImage = QImage(filename);
    atlasList.push_back(AtlasProperties(atlasImage,
                                        (float) 1.0 / atlasImage.width(),
                                        (float) 1.0 / atlasImage.height()));
    spriteToSync.push_back(SpriteSyncData());
    avgSpritePerSync.push_back(COARSE_MEM_SCALE_START);
    spriteTotal.push_back(0);

    return currentIndex;
}


key Machine::create(key atlasNum, BoundingBox loc){
    int currentIndex;
    float* texCoords;
    float  iw;
    float  ih;

    currentIndex = spriteList.size();

    loc.x1 += 1;
    loc.y1 += 1;
    spriteList.push_back(SpriteProperties(DRAW_FLAT, loc, atlasNum));

    texCoords = spriteList.last().texCoords;

    iw = atlasList[atlasNum].invW;
    ih = atlasList[atlasNum].invH;

    texCoords[0] = loc.x0 * iw;
    texCoords[1] = loc.y0 * ih;
    texCoords[2] = loc.x1 * iw;
    texCoords[3] = loc.y1 * ih;

    return currentIndex;
}

int Machine::getHeight(key num){
    return (spriteList[num].location.y1 - spriteList[num].location.y0) + 1;
}


int Machine::getWidth(key num){
    return (spriteList[num].location.x1 - spriteList[num].location.x0) + 1;
}

void Machine::setDrawHeight(key num, float h){
    spriteList[num].sz = QVector2D(spriteList[num].sz.x(), h);
}
void Machine::setDrawWidth(key num, float w){
    spriteList[num].sz = QVector2D(w, spriteList[num].sz.y());
}


void Machine::draw(key num, BoundingBox loc, float angle, float id, QVector4D hilight){
    SpriteSyncData*    props;
    VertexAttribArray* vdata;
    GLushort*          idata;
    SpriteProperties*  sprite_props;
    int                sliceW;
    int                sliceH;

    sprite_props = &spriteList[num];
    props = &spriteToSync[sprite_props->atlas];

    props->size++;
    if(props->size > props->capacity){
        props->capacity += COARSE_MEM_SCALE;
        props->data   = (VertexAttribArray*) realloc((void*) props->data,   (sizeof(VertexAttribArray)*4) * props->capacity);
        props->i_data = (GLushort*)          realloc((void*) props->i_data, (sizeof(GLushort)*4) * props->capacity);
    }

    vdata = props->data   + (props->size - 1) * 4;
    idata = props->i_data + (props->size - 1) * 4;

    sliceW = sprite_props->location.x1 - sprite_props->location.x0;
    sliceH = sprite_props->location.y1 - sprite_props->location.y0;

    if(sprite_props->flags & DRAW_TILED){
        vdata[0].p = QVector3D(loc.x1, loc.y0, UNIFORM_DEPTH);
        vdata[1].p = QVector3D(loc.x0, loc.y0, UNIFORM_DEPTH);
        vdata[2].p = QVector3D(loc.x0, loc.y1, UNIFORM_DEPTH);
        vdata[3].p = QVector3D(loc.x1, loc.y1, UNIFORM_DEPTH);
        vdata[0].s = QVector2D(((float) (loc.x1 - loc.x0)) / sliceW,
                               ((float) (loc.y1 - loc.y0)) / sliceH);
    }
    else{
        loc.x0 -= sprite_props->sz.x() * 0.5;
        loc.y0 -= sprite_props->sz.y() * 0.5;
        vdata[0].p = QVector3D(loc.x0 + sprite_props->sz.x(), loc.y0,                        UNIFORM_DEPTH);
        vdata[1].p = QVector3D(loc.x0,                        loc.y0,                        UNIFORM_DEPTH);
        vdata[2].p = QVector3D(loc.x0,                        loc.y0 + sprite_props->sz.y(), UNIFORM_DEPTH);
        vdata[3].p = QVector3D(loc.x0 + sprite_props->sz.x(), loc.y0 + sprite_props->sz.y(), UNIFORM_DEPTH);
        vdata[0].s = QVector2D(1, 1);
    }
    vdata[0].t = QVector2D(sprite_props->texCoords[2], sprite_props->texCoords[1]);
    vdata[1].t = QVector2D(sprite_props->texCoords[0], sprite_props->texCoords[1]);
    vdata[2].t = QVector2D(sprite_props->texCoords[0], sprite_props->texCoords[3]);
    vdata[3].t = QVector2D(sprite_props->texCoords[2], sprite_props->texCoords[3]);
    vdata[1].s = vdata[0].s;
    vdata[2].s = vdata[0].s;
    vdata[3].s = vdata[0].s;

    vdata[0].s_lo = QVector2D(sprite_props->texCoords[0], sprite_props->texCoords[1]);
    vdata[0].s_hi = QVector2D(sprite_props->texCoords[2] - sprite_props->texCoords[0],
                              sprite_props->texCoords[3] - sprite_props->texCoords[1]);
    vdata[1].s_lo = vdata[0].s_lo;
    vdata[1].s_hi = vdata[0].s_hi;
    vdata[2].s_lo = vdata[0].s_lo;
    vdata[2].s_hi = vdata[0].s_hi;
    vdata[3].s_lo = vdata[0].s_lo;
    vdata[3].s_hi = vdata[0].s_hi;

    vdata[0].o = (vdata[1].p + vdata[3].p) * 0.5;
    vdata[1].o = vdata[0].o;
    vdata[2].o = vdata[0].o;
    vdata[3].o = vdata[0].o;

    vdata[0].a = angle;
    vdata[1].a = angle;
    vdata[2].a = angle;
    vdata[3].a = angle;

    vdata[0].i = id;
    vdata[1].i = id;
    vdata[2].i = id;
    vdata[3].i = id;

    vdata[0].h = hilight;
    vdata[1].h = hilight;
    vdata[2].h = hilight;
    vdata[3].h = hilight;

    idata[0] = (props->size - 1) * 4;
    idata[1] = idata[0] + 1;
    idata[2] = idata[0] + 2;
    idata[3] = idata[0] + 3;

    spriteTotal[sprite_props->atlas]++;
}

void Machine::enableTile(key num){
    spriteList[num].flags = DRAW_TILED;
}

void Machine::disableTile(key num){
    spriteList[num].flags = DRAW_FLAT;
}

void Machine::setMemoryBeta(float b){
    linDec = b;
}

void Machine::sync(bool pick){
#define ADD_ATTRIBUTE_LOCATION(ASIZE, ALOC, OFFS, ATYPE)                                                     \
    (                                                                                                        \
        (parent->vertexAttribPointer(ALOC, ASIZE, GL_FLOAT, GL_FALSE, sizeof(ATYPE), (const void*) offset)), \
        (offset += OFFS),                                                                                    \
        (void)0                                                                                              \
    )

    int i;
    int size;

    const VertexAttribArray_Flat screen_quad_vert[] = {
        {QVector3D(                   0.0,                     0.0, 0.0), QVector2D(0.0, 0.0)},
        {QVector3D(parent->getDrawWidth(), parent->getDrawHeight(), 0.0), QVector2D(1.0, 1.0)},
        {QVector3D(                   0.0, parent->getDrawHeight(), 0.0), QVector2D(0.0, 1.0)},
        {QVector3D(parent->getDrawWidth(),                     0.0, 0.0), QVector2D(1.0, 0.0)}
    };

    static const GLuint screen_quad_ind[] = {
        0, 1, 2,
        3, 1, 0
    };

    GLuint   fbo;
    GLuint   renderBuff[2];
    GLenum   drawBuffers[2];

    GLuint   vbo[2];
    quintptr offset;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    offset = 0;
    parent->genBuffers(2, vbo);

    parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    ADD_ATTRIBUTE_LOCATION(3, attrLoc[A_POSITION], sizeof(QVector3D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(2, attrLoc[A_TEXCOORD], sizeof(QVector2D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(2, attrLoc[A_SCALE], sizeof(QVector2D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(2, attrLoc[A_SCALE_LO], sizeof(QVector2D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(2, attrLoc[A_SCALE_HI], sizeof(QVector2D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(1, attrLoc[A_ANGLE], sizeof(float), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(3, attrLoc[A_ORIGIN], sizeof(QVector3D), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(1, attrLoc[A_ID], sizeof(float), VertexAttribArray);
    ADD_ATTRIBUTE_LOCATION(4, attrLoc[A_HILIGHT], sizeof(QVector4D), VertexAttribArray);

    parent->genFramebuffers(1, &fbo);
    if(hasPickingTexture && pick) parent->deleteTexture(pickTexture);
    parent->genTextures(2, renderBuff);

    parent->_bindTexture(GL_TEXTURE_2D, renderBuff[0]);
    parent->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, parent->getDrawWidth(), parent->getDrawHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    parent->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    parent->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    parent->_bindTexture(GL_TEXTURE_2D, renderBuff[1]);
    parent->texImage2D(GL_TEXTURE_2D, 0, GL_R32F, parent->getDrawWidth(), parent->getDrawHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    parent->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    parent->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    parent->bindFramebuffer(GL_FRAMEBUFFER, fbo);
    parent->framebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderBuff[0], 0);
    parent->framebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, renderBuff[1], 0);

    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    drawBuffers[1] = GL_COLOR_ATTACHMENT1;
    parent->drawBuffers(2, drawBuffers);

    flat_shader.bind();
    flat_shader.setUniformValue("mvp_matrix", parent->getOrtho());
    flat_shader.setUniformValue("texture", 0);

    parent->bindFramebuffer(GL_FRAMEBUFFER, fbo);
    for(i = 0; i < spriteToSync.size(); i++){
        size = spriteToSync[i].size;
        if(size > 0){
            parent->activeTexture(GL_TEXTURE0);
            parent->bindTexture(atlasList[i].atlas);

            parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
            parent->bufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(VertexAttribArray), spriteToSync[i].data, GL_DYNAMIC_DRAW);

            parent->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
            parent->bufferData(GL_ELEMENT_ARRAY_BUFFER, size * 4 * sizeof(GLushort), spriteToSync[i].i_data, GL_DYNAMIC_DRAW);

            parent->drawElements(GL_QUADS, 4 * size, GL_UNSIGNED_SHORT, 0);

            if(spriteTotal[i] >= avgSpritePerSync[i]){
                avgSpritePerSync[i] = spriteTotal[i];
            }
            else{
                avgSpritePerSync[i] = ((float) avgSpritePerSync[i]) * linDec + ((float) spriteTotal[i]) * (1 - linDec);
            }
            spriteToSync[i].reset(avgSpritePerSync[i]);
            spriteTotal[i] = 0;
        }
    }
    parent->bindFramebuffer(GL_FRAMEBUFFER, 0);

    copy_shader.bind();
    copy_shader.setUniformValue("mvp_matrix", parent->getScreenMatrix());
    copy_shader.setUniformValue("texture", 0);

    parent->activeTexture(GL_TEXTURE0);
    parent->_bindTexture(GL_TEXTURE_2D, renderBuff[0]);

    parent->bindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    offset = 0;
    ADD_ATTRIBUTE_LOCATION(3, attrLoc[A_POSITION_FLAT], sizeof(QVector3D), VertexAttribArray_Flat);
    ADD_ATTRIBUTE_LOCATION(2, attrLoc[A_TEXCOORD_FLAT], sizeof(QVector2D), VertexAttribArray_Flat);
    parent->bufferData(GL_ARRAY_BUFFER, 4 * sizeof(VertexAttribArray_Flat), screen_quad_vert, GL_DYNAMIC_DRAW);

    parent->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    parent->bufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), screen_quad_ind, GL_DYNAMIC_DRAW);

    parent->drawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if(pick){
        hasPickingTexture = true;
        pickTexture = renderBuff[1];
        pickWidth  = parent->getDrawWidth();
        pickHeight = parent->getDrawHeight();
    } else{
        parent->deleteTexture(renderBuff[1]);
    }

    parent->deleteBuffers(2, vbo);
    parent->deleteTexture(renderBuff[0]);
    parent->deleteFramebuffers(1, &fbo);

}

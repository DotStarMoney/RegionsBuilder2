#include "glzmap.h"

glZMap::glZMap(QWidget *parent) :
    GLDrawSurface(parent)
{
}

glZMap::~glZMap(){
    delete spriteDraw;
}


void glZMap::initializeGL(){
    initializeOpenGLFunctions();

    _width = this->width();
    _height = this->height();

    glViewport(0, 0, this->getDrawWidth(), this->getDrawHeight());

    screen.setToIdentity();
    screen.ortho(0, this->getDrawWidth(), this->getDrawHeight(), 0, -1, 1);
    ortho = screen;
    qglClearColor(Qt::red);

    spriteDraw = new SpriteMachine::Machine(this);

    backAtlas = spriteDraw->loadAtlas(":/gfx/gfx/minimapback2.png");
    backSprite = spriteDraw->create(backAtlas, SpriteMachine::BoundingBox(0, 0, 101, 101));
    spriteDraw->enableTile(backSprite);

}

void glZMap::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT);

    spriteDraw->draw(backSprite, SpriteMachine::BoundingBox(0, 0, this->getDrawWidth(), this->getDrawHeight()));
    spriteDraw->sync();
}


void glZMap::resizeGL(int width, int height){
    _width = width;
    _height = height;
}

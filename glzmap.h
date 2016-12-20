#ifndef GLZMAP_H
#define GLZMAP_H

#include <QGLWidget>
#include "gldrawsurface.h"
#include "spritemachine.h"

class glZMap : public GLDrawSurface
{
    Q_OBJECT
public:
    explicit glZMap(QWidget *parent = 0);
    ~glZMap();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
private:
    SpriteMachine::Machine* spriteDraw;
    SpriteMachine::key     backSprite;
    SpriteMachine::key     backAtlas;


signals:

public slots:

};

#endif // GLZMAP_H

#ifndef GLMINIMAP_H
#define GLMINIMAP_H

#include <QGLWidget>
#include "gldrawsurface.h"
#include "spritemachine.h"

class glMiniMap : public GLDrawSurface
{
    Q_OBJECT
public:
    explicit glMiniMap(QWidget *parent = 0);
    ~glMiniMap();

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

#endif // GLMINIMAP_H

#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QGLShaderProgram>
#include <QString>
#include "gldrawsurface.h"
#include "spritemachine.h"
#include "primitivemachine.h"
#include "textmachine/textmachine.h"
#include "errlog.h"
#include "../../Includes/utils/RTree.h"

#define SCROLL_DRAG_WINDOW_EDGE 2
#define CAMERA_SCROLL_SPEED 8
#define MIN_SELECT_DIMENSION 2
#define COVERING_TRANSPARENCY 0.5
#define NUDGE_DIST 1

enum content_op{SOLO_SELECT, GROUP_SELECT, PAN, DRAG, NONE};
enum content_transform{SCALE_2, ROTATE, SCALE, NO_TRANSFORM};

struct Camera_t{
    QVector3D pos;
    QVector3D original_pos;
    QVector2D top_left;
    QVector2D bot_right;
};


struct MouseInfo_t{
    MouseInfo_t(){
        start = QVector2D(0,0);
        isPressed = false;
    }

    QVector2D start;
    QVector2D current;
    QVector2D snapped;
    bool      isPressed;
};

struct SnappingInfo_t{
    bool gridSnap;
    float xs;
    float ys;
};

struct entitySprite{
    entitySprite(){
        isCovering = false;
        isSelected = false;
        inTree = false;
    }
    entitySprite(float _angle,
                 int   _sprite,
                 int   _width,
                 int   _height,
                 QVector2D _pos,
                 int   _id = 32){
        ID = _id;
        angle = _angle;
        sprite = _sprite;
        width = _width;
        height = _height;
        pos = _pos;
        scalePoint = 0;
        isSelected = false;
        isCovering = false;
        inTree = false;
    }
    int locateMapping(int num){
        int i;
        for(i = 0; i < 4; i++){
            if(num == mapTable[i]) return i;
        }
        return -1;
    }


    bool  isCovering;
    bool  inTree;
    int   ID;
    int   drawOrderTag;
    float tl[2];
    float br[2];
    float angle;
    int   sprite;
    float width;
    float height;
    QVector2D pos;
    QVector2D boundPts[4];
    float grabRadius;
    float snapAngle;
    QVector2D snapPosition;
    float     snapWidth;
    float     snapHeight;
    int   scalePoint;
    int   mapTable[4];
    bool  isSelected;
};

struct ContentOp_t{
    ContentOp_t(){
        disableGroupSelect = false;
        operation = NONE;
        hasSolo = false;
    }
    QVector2D  mouseSelectStart;
    content_op operation;
    content_transform transform;
    int               transform_point;
    bool isTransforming;
    bool disableGroupSelect;
    bool hasSolo;
    entitySprite* selectedSprite;
    bool hasGroup;
    entitySprite selectGroupMetrics;
    bool angleSnapping;
};

template<typename T>
bool addToSelectedVector(T data, void* list){
    QVector<T>* list_d;
    list_d = (QVector<T>*) list;
    list_d->push_back(data);
    return true;
}

class MyGLWidget : public GLDrawSurface, ErrLog::Log
{
    Q_OBJECT
public:
    explicit MyGLWidget(QWidget *parent = 0);
    ~MyGLWidget();
    QString getSaveFileName();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent* );
    void keyReleaseEvent(QKeyEvent* );

private:
    GLuint back_texture;
    GLuint vboid[2];
    QGLShaderProgram default_prog;

    SpriteMachine::Machine* spriteDraw;
    PrimitiveMachine::Machine* lineDraw;
    TextMachine::Machine* textDraw;

    SpriteMachine::key      backTexture;
    SpriteMachine::key      backSprite;
    QString defaultFont_large;
    QString defaultFont;
    Camera_t camera;
    SnappingInfo_t snapping;

    MouseInfo_t mouse;
    ContentOp_t op;
    bool        CTRL_pressed;
    bool        SHIFT_pressed;
    bool        ALT_pressed;
    int         last_ID;

    SpriteMachine::key          testAtlas;
    QVector<SpriteMachine::key> spriteImages;
    QString saveFileName;
    bool    needsSaveFileName;
    QString curPalette;

    QHash<int, entitySprite> sprite;
    RTree<int, float, 2>     boundingBoxTree;
    QVector<int>             curSelected;
    QVector<entitySprite>    clipboard;
    QVector<int>             drawOrder;

    int  getID();
    void relinquishID(int i);

    void getMouseCurrent();
    void getCameraBounds();
    void doSelectBox(float selectBox[2][2]);
    void processSelection();
    void deselect();
    void setSelect();
    void resetCoverage();
    void modifyCurrentSelection(QVector2D transpose, content_transform trans = NO_TRANSFORM, int point = 0);
    void calculateGroupMetrics();
    void entityBound(entitySprite &s, int id, bool noSort = false, bool noTree = true);
    void updateBound(int spriten, bool noSort = false);
    void drawSelectionMarkings(entitySprite &s, float thickness = 1.5, bool noMarker = false, bool hilight = true);
    void determineTransform();
    void scrollOnExitBounds();
    void select_all();
    void copy(bool adjust = true);
    void cut();
    void stamp();
    void paste(bool atMouse = true, QVector2D offset = QVector2D(0,0));
    void selectionMoveBack();
    void selectionMoveForward();
    void sortCurrentSelected();
    QVector2D mouseTransformedCoords();
    float snapAngle(float angle);
    void drawSnapGuides();
    void saveRegion();
    void loadRegion();
    void loadPaletteItems(const QString &x, bool fakeSprites = true);


public slots:
    // ----- steez -----
    void loadPalette(const QString &x);
    void setGridSnap(bool s);
    void getSnapParameters(float xsnap, float ysnap);
    void saveRegionSlot();
    void loadRegionSlot();
    // -----------------

};

#endif // MYGLWIDGET_H

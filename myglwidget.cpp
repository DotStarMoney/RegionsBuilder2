#include "myglwidget.h"
#include <QDebug>
#include <QString>
#include <QVector3D>
#include <QVector2D>
#include "locale.h"
#include <QtAlgorithms>
#include <QMouseEvent>
#include <QTimer>
#include <QtMath>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include "vectorutility.h"

#define CORNER_GRAB_ROTATE_MIN_DIST 6
#define CORNER_GRAB_ROTATE_MAX_DIST 28

#define CORNER_GRAB_SCALE_DIST 6

#define SIDE_GRAB_SCALE_DIST 5

#define GROUP_CORNER_GRAB_ROTATE_MIN_DIST 10
#define GROUP_CORNER_GRAB_ROTATE_MAX_DIST 33

#define GROUP_CORNER_GRAB_SCALE_DIST 10
#define GROUP_SIDE_GRAB_SCALE_DIST 10

#define HILIGHT_COLOR  QVector4D(0.3922,0.6843,0.9994,1.0)
#define HILIGHT_COLOR2 QVector4D(0.9294,0.6843,0.4922,1.0)
#define LOLIGHT_COLOR  QVector4D(0.7,0.7,0.7,0.5)

MyGLWidget::MyGLWidget(QWidget *parent) :
    GLDrawSurface(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(18);
    op.hasGroup = false;
    op.isTransforming = false;
    op.operation = GROUP_SELECT;
    op.angleSnapping = false;
    SHIFT_pressed = false;
    CTRL_pressed = false;
    snapping.gridSnap = false;
    last_ID = 1;
    saveFileName = "";
    needsSaveFileName = true;
    curPalette = "";
}

MyGLWidget::~MyGLWidget(){
    delete spriteDraw;
    delete lineDraw;
}

void MyGLWidget::entityBound(entitySprite &s, int id, bool noSort, bool noTree){
    int q;
    float cosA;
    float sinA;
    QMatrix2x2 a, b;
    float matIn[4];
    float halfWidth;
    float old_tl[2];
    float old_br[2];
    float halfHeight;
    float largest;
    float dist;
    bool  noData;

    largest = 0;
    cosA = qCos(s.angle);
    sinA = qSin(s.angle);

    a.data()[0] = cosA; a.data()[1] = -sinA;
    a.data()[2] = sinA; a.data()[3] = cosA;
    a.copyDataTo(matIn);

    old_tl[0] = s.tl[0];
    old_tl[1] = s.tl[1];
    old_br[0] = s.br[0];
    old_br[1] = s.br[1];


    halfWidth = s.width * 0.5;
    halfHeight = s.height * 0.5;
    noData = true;
    for(q = 0; q < 4; q++){

        b.data()[0] = 0; b.data()[1] = halfWidth*(((q == 0) || (q == 3)) ? 1 : -1);
        b.data()[2] = 0; b.data()[3] = halfHeight*(((q == 2) || (q == 3)) ? 1 : -1);

        b = b * a;
        s.boundPts[q] = s.pos + QVector2D(b.data()[1], b.data()[3]);
        if(!noSort) s.mapTable[q] = q;

        dist = s.boundPts[q].distanceToPoint(s.pos);
        if(dist > largest){
            largest = dist;
        }

        if(noData){
            noData = false;
            s.tl[0] = s.boundPts[q].x();
            s.tl[1] = s.boundPts[q].y();
            s.br[0] = s.boundPts[q].x();
            s.br[1] = s.boundPts[q].y();
        }
        else{
            if(s.boundPts[q].x() < s.tl[0])
                s.tl[0] = s.boundPts[q].x();
            else if(s.boundPts[q].x() > s.br[0])
                s.br[0] = s.boundPts[q].x();

            if(s.boundPts[q].y() < s.tl[1])
                s.tl[1] = s.boundPts[q].y();
            else if(s.boundPts[q].y() > s.br[1])
                s.br[1] = s.boundPts[q].y();
        }
    }
    s.grabRadius = largest;

    if(!noTree){
        if(!s.inTree){
            boundingBoxTree.Insert(s.tl, s.br, id);
            s.inTree = true;
        }
        else{
            boundingBoxTree.Remove(old_tl, old_br, id);
            boundingBoxTree.Insert(s.tl, s.br, id);
        }
    }

    if(!noSort){
        for(q = 0; q < 4;){
            if(q == 0){
                q++;
            } else if((s.boundPts[q-1].y() < s.boundPts[q].y()) ||
                      ((s.boundPts[q-1].y() == s.boundPts[q].y()) && (s.boundPts[q-1].x() <= s.boundPts[q].x()))){
                q++;
            } else{
               qSwap(s.boundPts[q], s.boundPts[q - 1]);
               qSwap(s.mapTable[q], s.mapTable[q - 1]);
               q--;
            }
        }
    }
}

void MyGLWidget::updateBound(int spriten, bool noSort){
    if(!sprite.contains(spriten)) warning("sprite does not exist");
    entityBound(sprite[spriten], spriten, noSort, false);
}

void MyGLWidget::setGridSnap(bool s){
    snapping.gridSnap = s;
}


void MyGLWidget::determineTransform(){
    QVector2D    curPt;
    QVector2D    curDelta;
    QVector2D    prevEdge;
    QVector2D    nextEdge;
    entitySprite metrics;
    int i;
    int prevIndex;
    int nextIndex;
    double dist;
    float cgsd;
    float cgrmind;
    float cgrmaxd;
    float sgsd;

    if(op.hasSolo){
        metrics = *(op.selectedSprite);
        cgsd = CORNER_GRAB_SCALE_DIST;
        cgrmind = CORNER_GRAB_ROTATE_MIN_DIST;
        cgrmaxd = CORNER_GRAB_ROTATE_MAX_DIST;
        sgsd = SIDE_GRAB_SCALE_DIST;
    }
    else{
        metrics = op.selectGroupMetrics;
        cgsd = GROUP_CORNER_GRAB_SCALE_DIST;
        cgrmind = GROUP_CORNER_GRAB_ROTATE_MIN_DIST;
        cgrmaxd = GROUP_CORNER_GRAB_ROTATE_MAX_DIST;
        sgsd = GROUP_SIDE_GRAB_SCALE_DIST;
    }

    for(i = 0; i < 4; i++){
        curPt = metrics.boundPts[i];
        getMouseCurrent();
        curDelta = mouse.current - curPt;
        dist = curDelta.length();

        prevIndex = metrics.mapTable[i] - 1;
        if(prevIndex == -1) prevIndex = 3;
        prevIndex = metrics.locateMapping(prevIndex);

        nextIndex = metrics.mapTable[i] + 1;
        if(nextIndex == 4) nextIndex = 0;
        nextIndex = metrics.locateMapping(nextIndex);

        if(dist <= cgsd){
            op.transform       = SCALE_2;
            op.transform_point = metrics.mapTable[i];
            op.isTransforming  = true;
            break;
        }
        else if((dist > cgrmind) &&
                (dist <= cgrmaxd)){

            prevEdge = metrics.boundPts[prevIndex] - metrics.boundPts[i];
            nextEdge = metrics.boundPts[nextIndex] - metrics.boundPts[i];
            if((QVector2D::dotProduct(curDelta, prevEdge) < 0.0) &&
               (QVector2D::dotProduct(curDelta, nextEdge) < 0.0)){
                op.transform       = ROTATE;
                op.transform_point = metrics.mapTable[i];
                op.isTransforming  = true;
                break;
            }
        }

        curPt = (metrics.boundPts[nextIndex] + metrics.boundPts[i]) * 0.5;
        curDelta = mouse.current - curPt;
        dist = curDelta.length();
        if(dist <= sgsd){
            op.transform       = SCALE;
            op.transform_point = metrics.mapTable[i];
            op.isTransforming  = true;
        }
    }


}


void MyGLWidget::calculateGroupMetrics(){
    int i;

    if(curSelected.size() < 1) warning("nothing selected");

    op.selectGroupMetrics.angle = 0;
    op.selectGroupMetrics.ID = 0;
    op.selectGroupMetrics.tl[0] = sprite[curSelected[0]].tl[0];
    op.selectGroupMetrics.tl[1] = sprite[curSelected[0]].tl[1];
    op.selectGroupMetrics.br[0] = sprite[curSelected[0]].br[0];
    op.selectGroupMetrics.br[1] = sprite[curSelected[0]].br[1];
    for(i = 0; i < curSelected.size(); i++){
        if(sprite[curSelected[i]].tl[0] < op.selectGroupMetrics.tl[0]){
            op.selectGroupMetrics.tl[0] = sprite[curSelected[i]].tl[0];
        }
        if(sprite[curSelected[i]].br[0] > op.selectGroupMetrics.br[0]){
            op.selectGroupMetrics.br[0] = sprite[curSelected[i]].br[0];
        }
        if(sprite[curSelected[i]].tl[1] < op.selectGroupMetrics.tl[1]){
            op.selectGroupMetrics.tl[1] = sprite[curSelected[i]].tl[1];
        }
        if(sprite[curSelected[i]].br[1] > op.selectGroupMetrics.br[1]){
            op.selectGroupMetrics.br[1] = sprite[curSelected[i]].br[1];
        }

        op.selectGroupMetrics.pos.setX((op.selectGroupMetrics.tl[0] + op.selectGroupMetrics.br[0]) * 0.5);
        op.selectGroupMetrics.pos.setY((op.selectGroupMetrics.tl[1] + op.selectGroupMetrics.br[1]) * 0.5);
        op.selectGroupMetrics.width  = op.selectGroupMetrics.br[0] - op.selectGroupMetrics.tl[0];
        op.selectGroupMetrics.height = op.selectGroupMetrics.br[1] - op.selectGroupMetrics.tl[1];
    }
    entityBound(op.selectGroupMetrics, 0, false, true);
}


int MyGLWidget::getID(){
    int lastval;
    lastval = last_ID;
    last_ID++;
    return lastval;
}

void MyGLWidget::relinquishID(int i){
    //
}

QVector2D MyGLWidget::mouseTransformedCoords(){
    if(snapping.gridSnap && !ALT_pressed){
        return mouse.snapped;
    }
    else{
        return mouse.current;
    }
}

void MyGLWidget::mousePressEvent(QMouseEvent* e){
    int pxl;
    int i;
    int s;
    bool pickInGroup;

    mouse.start = mouse.current;

    getMouseCurrent();

    if(e->buttons() & Qt::LeftButton){
        mouse.isPressed = true;
        if(curSelected.size() != 0){
            op.isTransforming = false;
            determineTransform();
            if(op.isTransforming){
                op.disableGroupSelect = true;
                op.mouseSelectStart = mouseTransformedCoords();
                return;
            }
        }
        op.hasGroup = false;
        spriteDraw->transferPickTexture();
        pxl = (getDrawHeight() - e->y()) * getDrawWidth() + e->x();
        if(spriteDraw->pick(pxl) != 0){
            s = (int) spriteDraw->pick(pxl);
            pickInGroup = false;
            for(i = 0; i < curSelected.size(); i++){
                if(sprite[curSelected[i]].ID == s){
                    pickInGroup = true;
                    if(curSelected.size() > 1) op.hasGroup = true;
                    op.operation = DRAG;
                    op.disableGroupSelect = true;
                    break;
                }
            }
            if(!pickInGroup){
                if(!SHIFT_pressed){
                    deselect();
                    curSelected.push_back(s);
                    sprite[s].isSelected = true;
                    op.disableGroupSelect = true;
                    op.operation = SOLO_SELECT;
                    op.hasSolo = true;
                    op.selectedSprite = &sprite[s];
                }
                else{
                    curSelected.push_back(s);
                    sprite[s].isSelected = true;
                    op.disableGroupSelect = true;
                    op.operation = GROUP_SELECT;
                    op.hasSolo = false;
                    op.hasGroup = true;
                    calculateGroupMetrics();
                }
            }
        }
        else{
            if(!SHIFT_pressed) deselect();
            op.operation = GROUP_SELECT;
            op.hasSolo = false;
        }
        op.mouseSelectStart = mouseTransformedCoords();
    }
    else if(e->buttons() & Qt::RightButton){
        mouse.start = mouse.current;
        op.operation = PAN;
    }
}


void MyGLWidget::mouseReleaseEvent(QMouseEvent *){
    int i;
    mouse.isPressed = false;
    op.disableGroupSelect = false;
    if(op.operation == PAN) op.operation = NONE;
    if(op.operation == GROUP_SELECT){
        if(curSelected.size() == 1){
            op.operation = SOLO_SELECT;
            op.selectedSprite = &sprite[curSelected[0]];
            op.hasSolo = true;
        }
        else if(curSelected.size() > 1){
            op.hasGroup = true;
            if(!op.isTransforming && (op.operation != DRAG)) calculateGroupMetrics();
        }
    }
    if(op.isTransforming){
        for(i = 0; i < curSelected.size(); i++){
            sprite[curSelected[i]].snapAngle = sprite[curSelected[i]].angle;
            sprite[curSelected[i]].snapPosition = sprite[curSelected[i]].pos;
            sprite[curSelected[i]].snapWidth = sprite[curSelected[i]].width;
            sprite[curSelected[i]].snapHeight = sprite[curSelected[i]].height;
        }
    }
    op.isTransforming = false;

}

void MyGLWidget::mouseMoveEvent(QMouseEvent* e){


}

void MyGLWidget::initializeGL(){
    initializeOpenGLFunctions();

    glClearColor(1.0,0.0,0.0,1.0);
    spriteDraw = new SpriteMachine::Machine(this);
    lineDraw = new PrimitiveMachine::Machine(this);
    textDraw = new TextMachine::Machine(this);

    defaultFont_large = textDraw->loadFont(":/fonts/fonts/CourierNew_8");
    defaultFont       = textDraw->loadFont(":/fonts/fonts/MyriadPro_20");
    backTexture       = spriteDraw->loadAtlas(":/gfx/gfx/back.png");
    backSprite        = spriteDraw->create(backTexture, SpriteMachine::BoundingBox(0, 0, 48, 48));
    spriteDraw->enableTile(backSprite);

    camera.pos = QVector3D(0,0,0);

    setFocusPolicy(Qt::ClickFocus);

}

void MyGLWidget::loadPalette(const QString &x){
    loadPaletteItems(x, true);
}

void MyGLWidget::loadPaletteItems(const QString &x, bool fakeSprites){
    QFile datfile;
    QDataStream datstream;
    int i;
    qint32 count;
    SpriteMachine::BoundingBox b;
    entitySprite z;
    qint32 val;

    testAtlas = spriteDraw->loadAtlas((x + ".png").toStdString().c_str());
    datfile.setFileName(x);
    datfile.open(QIODevice::ReadOnly);
    datstream.setDevice(&datfile);
    datstream >> count;
    curPalette = x;

    for(i = 0; i < count; i++){
        datstream >> val; b.x0 = val;
        datstream >> val; b.y0 = val;
        datstream >> val; b.x1 = val;
        datstream >> val; b.y1 = val;
        b.x1 += b.x0 - 1;
        b.y1 += b.y0 - 1;
        spriteImages.push_back(spriteDraw->create(testAtlas, b));
    }
    datfile.close();

    if(fakeSprites){
        for(i = 0; i < spriteImages.size(); i++){
            z.angle = 0;
            z.snapAngle = z.angle;
            z.sprite = spriteImages[i];
            z.width = spriteDraw->getWidth(spriteImages[i]);
            z.height = spriteDraw->getHeight(spriteImages[i]);
            z.snapWidth = z.width;
            z.snapHeight = z.height;
            z.pos = QVector2D((qrand() % getDrawWidth()) - getDrawWidth() * 0.5, (qrand() % getDrawHeight()) - getDrawHeight() * 0.5);
            z.pos = QVector2D(((int)(z.pos.x() / snapping.xs)) * snapping.xs, ((int)(z.pos.y() / snapping.ys)) * snapping.ys);
            z.snapPosition = z.pos;
            z.drawOrderTag = i;
            z.ID = getID();
            z.isSelected = false;
            drawOrder.push_back(z.ID);
            sprite.insert(z.ID, z);
            updateBound(z.ID);
        }
    }
}

void MyGLWidget::getMouseCurrent(){
    mouse.current = QVector2D(this->mapFromGlobal(QCursor::pos())) + QVector2D(camera.pos.x(), camera.pos.y());
    mouse.current = mouse.current - QVector2D(getDrawWidth() * 0.5, getDrawHeight() * 0.5);
    mouse.snapped = QVector2D(((int) ((mouse.current.x() + snapping.xs*0.5) / snapping.xs)) * snapping.xs,
                              ((int) ((mouse.current.y() + snapping.xs*0.5) / snapping.ys)) * snapping.ys);

}

void MyGLWidget::getCameraBounds(){
    camera.top_left  = QVector2D(camera.pos.x(), camera.pos.y()) - QVector2D(getDrawWidth() * 0.5, getDrawHeight() * 0.5);
    camera.bot_right = QVector2D(camera.pos.x(), camera.pos.y()) + QVector2D(getDrawWidth() * 0.5, getDrawHeight() * 0.5);
}

void MyGLWidget::doSelectBox(float selectBox[2][2]){
    int i, q;
    float xx[2][4];
    float yy[2][4];
    float actualBox[2][2];
    QVector<int> prevSelect;
    QHash<int, int> curHas;

    if(mouse.isPressed){
        if(QVector2D(mouse.start.x() - mouse.current.x(), mouse.start.y() - mouse.current.y()).length() < MIN_SELECT_DIMENSION) return;
        if(mouse.current.x() > (camera.bot_right.x() - SCROLL_DRAG_WINDOW_EDGE)){
            selectBox[1][0] = (camera.bot_right.x() - SCROLL_DRAG_WINDOW_EDGE);
            selectBox[0][0] = (qMax(mouse.start.x(), camera.top_left.x() - SCROLL_DRAG_WINDOW_EDGE));
            camera.pos = camera.pos + QVector3D(CAMERA_SCROLL_SPEED,0.0,0.0);
        }
        else if(mouse.current.x() < (camera.top_left.x() + SCROLL_DRAG_WINDOW_EDGE)){
            selectBox[0][0] = (camera.top_left.x() + SCROLL_DRAG_WINDOW_EDGE);
            selectBox[1][0] = (qMin(mouse.start.x(), camera.bot_right.x() + SCROLL_DRAG_WINDOW_EDGE));
            camera.pos = camera.pos - QVector3D(CAMERA_SCROLL_SPEED,0.0,0.0);
        }
        else{
            if(mouse.current.x() < mouse.start.x()){
                selectBox[0][0] = (mouse.current.x());
                selectBox[1][0] = (qMin(mouse.start.x(), camera.bot_right.x() + SCROLL_DRAG_WINDOW_EDGE));
            }
            else{
                selectBox[1][0] = (mouse.current.x());
                selectBox[0][0] = (qMax(mouse.start.x(), camera.top_left.x() - SCROLL_DRAG_WINDOW_EDGE));
            }
        }

        if(mouse.current.y() > (camera.bot_right.y() - SCROLL_DRAG_WINDOW_EDGE)){
            selectBox[1][1] = (camera.bot_right.y() - SCROLL_DRAG_WINDOW_EDGE);
            selectBox[0][1] = (qMax(mouse.start.y(), camera.top_left.y() - SCROLL_DRAG_WINDOW_EDGE));
            camera.pos = camera.pos + QVector3D(0.0,CAMERA_SCROLL_SPEED,0.0);
        }
        else if(mouse.current.y() < (camera.top_left.y() + SCROLL_DRAG_WINDOW_EDGE)){
            selectBox[0][1] = (camera.top_left.y() + SCROLL_DRAG_WINDOW_EDGE);
            selectBox[1][1] = (qMin(mouse.start.y(), camera.bot_right.y() + SCROLL_DRAG_WINDOW_EDGE));
            camera.pos = camera.pos - QVector3D(0.0,CAMERA_SCROLL_SPEED,0.0);
        }
        else{
            if(mouse.current.y() < mouse.start.y()){
                selectBox[0][1] = (mouse.current.y());
                selectBox[1][1] = (qMin(mouse.start.y(), camera.bot_right.y() + SCROLL_DRAG_WINDOW_EDGE));
            }
            else{
                selectBox[1][1] = (mouse.current.y());
                selectBox[0][1] = (qMax(mouse.start.y(), camera.top_left.y() - SCROLL_DRAG_WINDOW_EDGE));
            }
        }        
        if(SHIFT_pressed){
            for(i = 0; i < curSelected.size(); i++){
                prevSelect.push_back(curSelected[i]);
            }
        }
        deselect();
        actualBox[0][0] = mouse.start.x();
        actualBox[0][1] = mouse.start.y();
        actualBox[1][0] = mouse.current.x();
        actualBox[1][1] = mouse.current.y();
        if(actualBox[0][0] > actualBox[1][0]) qSwap(actualBox[0][0], actualBox[1][0]);
        if(actualBox[0][1] > actualBox[1][1]) qSwap(actualBox[0][1], actualBox[1][1]);


        boundingBoxTree.Search(actualBox[0], actualBox[1], addToSelectedVector<int>, (void*) &curSelected);
        for(i = 0; i < curSelected.size();){
            if(!VectorUtility::rotatedRectangleIntersection(QVector2D((actualBox[1][0] + actualBox[0][0]) * 0.5,
                                                                     (actualBox[1][1] + actualBox[0][1]) * 0.5),
                                                           QVector2D(actualBox[1][0] - actualBox[0][0] + 1,
                                                                     actualBox[1][1] - actualBox[0][1] + 1),
                                                           0.0,
                                                           sprite[curSelected[i]].pos,
                                                           QVector2D(sprite[curSelected[i]].width, sprite[curSelected[i]].height),
                                                           sprite[curSelected[i]].angle, xx, yy)){
                curSelected.remove(i);
            }
            else{
                curHas.insert(curSelected[i], curSelected[i]);
                i++;
            }
        }
        for(i = 0; i < prevSelect.size(); i++){
            if(!curHas.contains(prevSelect[i])){
                curSelected.push_back(prevSelect[i]);
            }
        }

        setSelect();
        lineDraw->drawDottedBox(QVector2D(selectBox[0][0], selectBox[0][1]), QVector2D(selectBox[1][0], selectBox[1][1]), QVector4D(0.8,0.8,0.8,0.8), 1.5, 3);

    }
}

#define GRAB_CIRCLE_RADIUS 4
void MyGLWidget::drawSelectionMarkings(entitySprite &s, float thickness, bool noMarker, bool hilight){
    int q;
    QVector2D pts[5];
    pts[0] = s.boundPts[0];
    pts[1] = s.boundPts[1];
    pts[2] = s.boundPts[3];
    pts[3] = s.boundPts[2];
    pts[4] = pts[0];
    lineDraw->drawPolyline(5, pts, LOLIGHT_COLOR, thickness, 2.5);

    if(noMarker) return;
    lineDraw->drawCircle(s.pos, 5, (hilight ? HILIGHT_COLOR : LOLIGHT_COLOR), 2, 2.5);
    lineDraw->drawCircle(s.pos, 8, (hilight ? HILIGHT_COLOR : LOLIGHT_COLOR), 2, 2.5);


    for(q = 0; q < 4; q++){
        lineDraw->drawCircle(pts[q], GRAB_CIRCLE_RADIUS + thickness, HILIGHT_COLOR, 1.5, 2.5);
    }
    for(q = 0; q < 4; q++){
        lineDraw->drawRotatedBox((pts[q] + pts[q + 1]) * 0.5, GRAB_CIRCLE_RADIUS + thickness, s.angle + (PI/2) * q, (hilight ? HILIGHT_COLOR2 : LOLIGHT_COLOR), 1.5, 2.5);
    }
}

void MyGLWidget::sortCurrentSelected(){
    int q;
    for(q = 0; q < curSelected.size();){
        if(q == 0){
            q++;
        } else if(sprite[curSelected[q - 1]].drawOrderTag < sprite[curSelected[q]].drawOrderTag){
            q++;
        } else{
           qSwap(curSelected[q - 1], curSelected[q]);
           q--;
        }
    }
}

void MyGLWidget::selectionMoveBack(){
    int i;
    int orderID;
    int curLimit;

    curLimit = 0;
    sortCurrentSelected();

    for(i = 0; i < curSelected.size(); i++){
        orderID = sprite[curSelected[i]].drawOrderTag;
        if((orderID > curLimit) && (sprite.size() > 1)){
            qSwap(drawOrder[orderID], drawOrder[orderID - 1]);
            sprite[curSelected[i]].drawOrderTag -= 1;
            sprite[drawOrder[orderID]].drawOrderTag += 1;
        }
        else if(orderID <= curLimit){
            curLimit++;
        }
    }
}

QString MyGLWidget::getSaveFileName(){
    return saveFileName;
}


void MyGLWidget::saveRegion(){
    QString     dialogRet;
    QDataStream saveFile;
    QFile       datFile;
    QHash<int, entitySprite>::iterator iS;
    entitySprite cur;

    if((saveFileName == "") || needsSaveFileName){
        dialogRet = QFileDialog::getSaveFileName(this, tr("Save Region As"), "", tr("Regions Builder 1.2 Data (*.rgn)"));
        if(dialogRet != ""){
            saveFileName = dialogRet;
            needsSaveFileName = false;
        }
    }
    if(saveFileName == "") return;
    if(curPalette == "") return;


    datFile.setFileName(saveFileName);
    datFile.open(QIODevice::WriteOnly);
    saveFile.setDevice(&datFile);

    saveFile << curPalette.toUtf8().constData();

    saveFile << ((int) sprite.size());
    for(iS = sprite.begin(); iS != sprite.end(); iS++){
        cur = *iS;
        saveFile << cur.ID;
        saveFile << cur.sprite;
        saveFile << cur.pos.x();
        saveFile << cur.pos.y();
        saveFile << cur.angle;
        saveFile << cur.width;
        saveFile << cur.height;
        saveFile << cur.drawOrderTag;
    }
    datFile.close();


}

void MyGLWidget::loadRegion(){
    // prompt dialog and load
    QString     dialogRet;
    QDataStream loadFile;
    QFile       datFile;
    entitySprite cur;
    float x;
    float y;
    int numSprites;
    int i;
    char* palName;

    dialogRet = QFileDialog::getOpenFileName(this, tr("Open Region"), "", tr("Regions Builder 1.2 Region (*.rgn)"));
    if(dialogRet != ""){
        saveFileName = dialogRet;
        needsSaveFileName = false;
    }
    else{
        return;
    }

    select_all();
    cut();

    datFile.setFileName(saveFileName);
    datFile.open(QIODevice::ReadOnly);
    loadFile.setDevice(&datFile);

    palName = new char[256];
    loadFile >> palName;

    loadPaletteItems(QString(palName), false);

    loadFile >> numSprites;
    drawOrder.resize(numSprites);
    qDebug() << numSprites;
    sprite.clear();
    for(i = 0; i < numSprites; i++){
        loadFile >> cur.ID;
        loadFile >> cur.sprite;
        loadFile >> x;
        loadFile >> y;
        cur.pos = QVector2D(x, y);
        loadFile >> cur.angle;
        loadFile >> cur.width;
        loadFile >> cur.height;
        loadFile >> cur.drawOrderTag;

        cur.ID = getID();
        qDebug() << cur.ID;

        drawOrder[cur.drawOrderTag] = cur.ID;
        cur.snapAngle = cur.angle;
        cur.snapWidth = cur.width;
        cur.snapHeight = cur.height;
        cur.snapPosition = cur.pos;
        cur.isSelected = false;
        sprite.insert(cur.ID, cur);
        updateBound(cur.ID);
    }
    datFile.close();

    qDebug() << "hash data";
    QHash<int, entitySprite>::iterator iS;
    for(iS = sprite.begin(); iS != sprite.end(); iS++){
        qDebug() << (*iS).ID;
    }


    op.hasSolo = false;
    op.hasGroup = false;
    op.disableGroupSelect = false;
    op.operation = GROUP_SELECT;
    delete palName;
}

void MyGLWidget::saveRegionSlot(){
    needsSaveFileName = true;
    saveRegion();
}

void MyGLWidget::loadRegionSlot(){
    loadRegion();
}

void MyGLWidget::selectionMoveForward(){
    int i;
    int orderID;
    int curLimit;

    curLimit = drawOrder.size() - 1;
    sortCurrentSelected();
    for(i = 0; i < ((int) (curSelected.size() * 0.5)); i++){
        qSwap(curSelected[i], curSelected[curSelected.size() - 1 - i]);
    }

    for(i = 0; i < curSelected.size(); i++){
        orderID = sprite[curSelected[i]].drawOrderTag;
        if((orderID < curLimit) && (sprite.size() > 1)){
            qSwap(drawOrder[orderID], drawOrder[orderID + 1]);
            sprite[curSelected[i]].drawOrderTag += 1;
            sprite[drawOrder[orderID]].drawOrderTag -= 1;
        }
        else if(orderID >= curLimit){
            curLimit--;
        }
    }
}


void MyGLWidget::processSelection(){
    int i;
    entitySprite curSprite;
    float xx[2][4];
    float yy[2][4];
    QVector<int> coveringSprites;
    bool mark;


    if(op.hasGroup)
        mark = true;
    else
        mark = false;

    if(curSelected.size() > 0){
        for(i = 0; i < curSelected.size(); i++){
            curSprite = sprite.value(curSelected[i]);
            drawSelectionMarkings(curSprite, 1.5, mark);
        }
    }
    if(op.hasGroup){
        drawSelectionMarkings(op.selectGroupMetrics, 5, false, false);
    }else{
        if(op.isTransforming && (op.transform == ROTATE)){
            lineDraw->drawCircle(op.selectedSprite->pos, op.selectedSprite->grabRadius, QVector4D(0.4,0.4,0.4,0.4), 1.5);
        }
    }

    resetCoverage();
    if(op.hasSolo){
        boundingBoxTree.Search(op.selectedSprite->tl, op.selectedSprite->br, addToSelectedVector<int>, (void*) &coveringSprites);
        for(i = 0; i < coveringSprites.size();){
            if(!VectorUtility::rotatedRectangleIntersection(op.selectedSprite->pos,
                                                            QVector2D(op.selectedSprite->width, op.selectedSprite->height),
                                                            op.selectedSprite->angle,
                                                            sprite[coveringSprites[i]].pos,
                                                            QVector2D(sprite[coveringSprites[i]].width, sprite[coveringSprites[i]].height),
                                                            sprite[coveringSprites[i]].angle, xx, yy)){


                coveringSprites.remove(i);
            }
            else{
                i++;
            }
        }
        for(i = 0; i < coveringSprites.size(); i++){
            if(coveringSprites[i] != op.selectedSprite->ID){
                sprite[coveringSprites[i]].isCovering = true;
            }
            else{
                sprite[coveringSprites[i]].isCovering = false;
            }
        }
    }
    if(mouse.isPressed && (op.hasGroup || op.hasSolo) && !op.isTransforming){
        op.operation = DRAG;
        modifyCurrentSelection(mouseTransformedCoords() - op.mouseSelectStart);
        op.mouseSelectStart = mouseTransformedCoords();
        getCameraBounds();
        getMouseCurrent();
        scrollOnExitBounds();
    }


}

void MyGLWidget::resetCoverage(){
    QHash<int, entitySprite>::iterator i;
    for(i = sprite.begin(); i != sprite.end(); i++){
        (*i).isCovering = false;
    }
}

void MyGLWidget::deselect(){
    int i;
    for(i = 0; i < curSelected.size(); i++){
        sprite[curSelected[i]].isSelected = false;
    }
    curSelected.clear();
}

void MyGLWidget::setSelect(){
    int i;
    for(i = 0; i < curSelected.size(); i++){
        sprite[curSelected[i]].isSelected = true;
    }
}

float MyGLWidget::snapAngle(float angle){
    // snap divisions;
#define ROTATION_SNAP_DIVISIONS 16.0

    float sliceSize;
    float curAngle;
    int i;

    while(angle >= (VU_PI*2)) angle -= VU_PI*2;
    while(angle < 0) angle += VU_PI*2;
    if(op.angleSnapping) return angle;

    sliceSize = (2*VU_PI) / ((float) ROTATION_SNAP_DIVISIONS);
    for(i = 1; i < ROTATION_SNAP_DIVISIONS; i++){
        curAngle = sliceSize * i;
        if((angle > (curAngle - sliceSize*0.5)) && (angle <= (curAngle + sliceSize*0.5))){
            return curAngle;
        }
    }

    return 0;
}

void MyGLWidget::modifyCurrentSelection(QVector2D transpose, content_transform trans, int point){
    int i, q;
    QVector2D prevOppVec;
    QVector2D nextOppVec;
    QVector2D scaleVec;
    QVector2D newPos;
    QVector2D rotVec;
    int prevIndex;
    int nextIndex;
    int oppIndex;
    float refAngle;
    float scaleDim[2];
    float origAngle;
    float newAngle;
    float diffAngle;
    entitySprite metrics;


    for(i = 0; i < curSelected.size(); i++){
        sprite[curSelected[i]].pos = sprite[curSelected[i]].pos + transpose;
        sprite[curSelected[i]].snapPosition = sprite[curSelected[i]].pos;
        updateBound(curSelected[i]);
    }
    if(op.hasGroup){
        op.selectGroupMetrics.pos = op.selectGroupMetrics.pos + transpose;
        entityBound(op.selectGroupMetrics, 0, false, true);
        metrics = op.selectGroupMetrics;
    }
    else{
        metrics = *op.selectedSprite;
    }

    if(trans != NO_TRANSFORM){

        prevIndex = point - 1;
        if(prevIndex < 0) prevIndex = 3;
        prevIndex = metrics.locateMapping(prevIndex);

        nextIndex = (point + 1) % 4;
        nextIndex = metrics.locateMapping(nextIndex);

        oppIndex = (point + 2) % 4;
        oppIndex = metrics.locateMapping(oppIndex);

        prevOppVec = metrics.boundPts[prevIndex] - metrics.boundPts[oppIndex];
        nextOppVec = metrics.boundPts[nextIndex] - metrics.boundPts[oppIndex];

        prevOppVec.normalize();
        nextOppVec.normalize();
        if((point % 2) == 0){
            prevOppVec = prevOppVec * ((metrics.width < 0) ? -1 : 1);
            nextOppVec = nextOppVec * ((metrics.height < 0) ? -1 : 1);
        }
        else{
            prevOppVec = prevOppVec * ((metrics.height < 0) ? -1 : 1);
            nextOppVec = nextOppVec * ((metrics.width < 0) ? -1 : 1);
        }

        switch(trans){
        case SCALE_2:

            scaleVec = mouseTransformedCoords() - metrics.boundPts[oppIndex];

            scaleDim[0] = QVector2D::dotProduct(prevOppVec, scaleVec);
            scaleDim[1] = QVector2D::dotProduct(nextOppVec, scaleVec);
            if((point % 2) == 0 ){
                qSwap(scaleDim[0], scaleDim[1]);
            }

            newPos = (mouseTransformedCoords() + metrics.boundPts[oppIndex]) * 0.5;
            break;
        case SCALE:
            scaleVec = mouseTransformedCoords() -
                       (metrics.boundPts[metrics.locateMapping(point)] +
                       metrics.boundPts[nextIndex]) * 0.5;
            if((point % 2) == 0){
                scaleDim[0] = metrics.height + QVector2D::dotProduct(nextOppVec, scaleVec);
                scaleDim[1] = metrics.width;
            }
            else{
                scaleDim[1] = metrics.width + QVector2D::dotProduct(nextOppVec, scaleVec);
                scaleDim[0] = metrics.height;
            }
            newPos = metrics.pos + QVector2D::dotProduct(nextOppVec, scaleVec) * (nextOppVec * 0.5);

            break;
        case ROTATE:
            origAngle = atan2(op.mouseSelectStart.y() - metrics.pos.y(), op.mouseSelectStart.x() - metrics.pos.x());
            newAngle  = atan2(mouse.current.y() - metrics.pos.y(), mouse.current.x() - metrics.pos.x());
            diffAngle = newAngle - origAngle;
            break;
        }

        if(op.hasSolo){
            if((trans == SCALE) || (trans == SCALE_2)){
                op.selectedSprite->width  = scaleDim[1];
                op.selectedSprite->height = scaleDim[0];
                op.selectedSprite->snapWidth  = scaleDim[1];
                op.selectedSprite->snapHeight = scaleDim[0];
                op.selectedSprite->pos = newPos;
                op.selectedSprite->snapPosition = newPos;
            }else{
                op.selectedSprite->angle     = snapAngle(op.selectedSprite->snapAngle + diffAngle);
                op.selectedSprite->snapAngle = op.selectedSprite->snapAngle + diffAngle;
                op.mouseSelectStart = mouse.current;
            }
            updateBound(op.selectedSprite->ID);
        }else if(op.hasGroup){
            if((trans == SCALE) || (trans == SCALE_2)){

            }else{
                metrics.snapAngle = metrics.snapAngle + diffAngle;
                refAngle = snapAngle(metrics.snapAngle) - metrics.angle;
                metrics.angle = snapAngle(metrics.snapAngle);
                for(i = 0; i < curSelected.size(); i++){

                    rotVec = sprite[curSelected[i]].snapPosition - metrics.pos;

                    newAngle = atan2(rotVec.y(), rotVec.x()) + refAngle;
                    sprite[curSelected[i]].pos          = metrics.pos + QVector2D(cos(newAngle), qSin(newAngle)) * rotVec.length();
                    sprite[curSelected[i]].snapPosition = metrics.pos + QVector2D(cos(newAngle), qSin(newAngle)) * rotVec.length();

                    newAngle = sprite[curSelected[i]].snapAngle + refAngle;

                    sprite[curSelected[i]].angle = newAngle;
                    sprite[curSelected[i]].snapAngle = newAngle;

                    updateBound(curSelected[i]);

                }
                op.mouseSelectStart = mouse.current;
                op.selectGroupMetrics = metrics;
                entityBound(op.selectGroupMetrics, 0);
            }
        }
    }
}

void MyGLWidget::scrollOnExitBounds(){
    if(mouse.current.x() < camera.top_left.x())
        camera.pos.setX(camera.pos.x() - CAMERA_SCROLL_SPEED);
    else if(mouse.current.x() > camera.bot_right.x())
        camera.pos.setX(camera.pos.x() + CAMERA_SCROLL_SPEED);

    if(mouse.current.y() < camera.top_left.y())
        camera.pos.setY(camera.pos.y() - CAMERA_SCROLL_SPEED);
    else if(mouse.current.y() > camera.bot_right.y())
        camera.pos.setY(camera.pos.y() + CAMERA_SCROLL_SPEED);
    getCameraBounds();
}

void MyGLWidget::keyPressEvent(QKeyEvent* k){
    int i;
    QVector2D centroid;
    if(k->key() == Qt::Key_Up){
        modifyCurrentSelection(QVector2D(0, -NUDGE_DIST));
    }
    else if(k->key() == Qt::Key_Right){
        modifyCurrentSelection(QVector2D(NUDGE_DIST, 0));
    }
    else if(k->key() == Qt::Key_Down){
        modifyCurrentSelection(QVector2D(0, NUDGE_DIST));
    }
    else if(k->key() == Qt::Key_Left){
        modifyCurrentSelection(QVector2D(-NUDGE_DIST, 0));
    }
    else if(k->key() == Qt::Key_Control){
        CTRL_pressed = true;
    }
    else if(k->key() == Qt::Key_Shift){
        SHIFT_pressed = true;
    }
    else if(k->key() == Qt::Key_A){
        if(CTRL_pressed){
            select_all();
        }
    }
    else if(k->key() == Qt::Key_X){
        if(CTRL_pressed){
            copy();
            cut();
        }
    }
    else if(k->key() == Qt::Key_C){
        if(CTRL_pressed){
            copy();
        }
    }
    else if(k->key() == Qt::Key_V){
        if(CTRL_pressed){
            paste();
        }
    }
    else if(k->key() == Qt::Key_Alt){
        ALT_pressed = true;
        op.angleSnapping = true;
    }
    else if(k->key() == Qt::Key_BracketLeft){
        if(CTRL_pressed){
            selectionMoveBack();
        }
    }
    else if(k->key() == Qt::Key_BracketRight){
        if(CTRL_pressed){
            selectionMoveForward();
        }
    }
    else if(k->key() == Qt::Key_Delete){
        cut();
    }
    else if(k->key() == Qt::Key_S){
        if(CTRL_pressed){
            saveRegion();
        }
    }
    else if(k->key() == Qt::Key_Space){
        stamp();
    }
}

void MyGLWidget::stamp(){
    int i;
    QVector2D centroid;
    if(op.operation == DRAG){
        copy();
        getMouseCurrent();
        if(op.hasSolo){
            paste(true, -mouseTransformedCoords() + op.selectedSprite->pos);
        }
        else{
            centroid = QVector2D(0,0);
            for(i = 0; i < curSelected.size(); i++){
                centroid = centroid + sprite[curSelected[i]].pos;
            }
            centroid = centroid * (1.0 / ((float) curSelected.size()));
            paste(true, -mouseTransformedCoords() + centroid);
        }
    }
}

void MyGLWidget::keyReleaseEvent(QKeyEvent *k){
    if(k->key() == Qt::Key_Control){
        CTRL_pressed = false;
    }
    else if(k->key() == Qt::Key_Shift){
        SHIFT_pressed = false;
    }
    else if(k->key() == Qt::Key_Alt){
        ALT_pressed = false;
        op.angleSnapping = false;
    }
}

void MyGLWidget::select_all(){
    QHash<int, entitySprite>::iterator s_i;
    entitySprite curSprite;
    deselect();
    for(s_i = sprite.begin(); s_i != sprite.end(); s_i++){
        curSprite = *s_i;
        curSelected.push_back(curSprite.ID);
    }
    setSelect();
    if(curSelected.size() == 1){
        op.hasSolo = true;
        op.hasGroup = false;
    }
    else if(curSelected.size() > 1){
        op.hasSolo = false;
        op.hasGroup = true;
        calculateGroupMetrics();
    }
}

void MyGLWidget::copy(bool adjust){
    int i;
    entitySprite s;
    QVector2D centroid;
    clipboard.clear();
    centroid = QVector2D(0, 0);
    for(i = 0; i < curSelected.size(); i++){
        s = sprite.value(curSelected[i]);
        s.isSelected = false;
        s.inTree = false;
        clipboard.push_back(s);
        centroid = centroid + clipboard[i].pos;
    }
    centroid = centroid * (1.0 / ((float) clipboard.size()));

    for(i = 0; i < clipboard.size(); i++){
        clipboard[i].pos = clipboard[i].pos - centroid;
        clipboard[i].snapPosition = clipboard[i].pos;
    }

    for(i = 0; i < clipboard.size();){
        if(i == 0){
            i++;
        } else if(clipboard[i-1].drawOrderTag < clipboard[i].drawOrderTag){
            i++;
        } else{
           qSwap(clipboard[i], clipboard[i-1]);
           i--;
        }
    }

}

void MyGLWidget::cut(){
    int i;
    int updateVals;
    entitySprite s;

    for(i = 0; i < curSelected.size(); i++){
        relinquishID(curSelected[i]);
        boundingBoxTree.Remove(sprite[curSelected[i]].tl, sprite[curSelected[i]].br, curSelected[i]);
        drawOrder.remove(sprite[curSelected[i]].drawOrderTag);
        for(updateVals = sprite[curSelected[i]].drawOrderTag; updateVals < drawOrder.size(); updateVals++){
            sprite[drawOrder[updateVals]].drawOrderTag = updateVals;
        }
        sprite.remove(curSelected[i]);
    }
    op.hasGroup = false;
    op.hasSolo = false;
    op.operation = GROUP_SELECT;
    curSelected.clear();
}

void MyGLWidget::paste(bool atMouse, QVector2D offset){
    int i;
    entitySprite s;
    if(clipboard.size() == 0) warning("nothing to paste");
    getMouseCurrent();
    deselect();
    for(i = 0; i < clipboard.size(); i++){
        s = clipboard.value(i);
        if(atMouse){
            s.pos = s.pos + mouseTransformedCoords();
        }
        else{
            s.pos = s.pos + QVector2D(camera.pos.x(), camera.pos.y());
        }
        s.pos = s.pos + offset;
        s.ID = getID();
        s.drawOrderTag = drawOrder.size();
        drawOrder.push_back(s.ID);
        sprite.insert(s.ID, s);
        curSelected.push_back(s.ID);
        updateBound(s.ID);
    }


    if(clipboard.size() > 1){
        op.hasGroup = true;
        op.hasSolo = false;
        op.operation = GROUP_SELECT;
        calculateGroupMetrics();
    }
    else{
        op.hasGroup = false;
        op.hasSolo = true;
        op.selectedSprite = &sprite[s.ID];
        op.operation = SOLO_SELECT;
    }
    setSelect();

}

void MyGLWidget::getSnapParameters(float xsnap, float ysnap){
    snapping.xs = xsnap;
    snapping.ys = ysnap;
}


void MyGLWidget::drawSnapGuides(){
    float xOffset;
    float yOffset;

    if(snapping.gridSnap){
        xOffset = ((int) (camera.top_left.x() / snapping.xs)) * snapping.xs;
        yOffset = ((int) (camera.top_left.y() / snapping.ys)) * snapping.ys;

        while(xOffset <= camera.bot_right.x()){
            lineDraw->drawLine(QVector2D(xOffset, camera.top_left.y()),
                               QVector2D(xOffset, camera.bot_right.y()),
                               QVector4D(0.5, 0.5, 1.0, 0.2));
            xOffset += snapping.xs;
        }

        while(yOffset <= camera.bot_right.y()){
            lineDraw->drawLine(QVector2D(camera.top_left.x(), yOffset),
                               QVector2D(camera.bot_right.x(), yOffset),
                               QVector4D(0.5, 0.5, 1.0, 0.2));
            yOffset += snapping.ys;
        }
    }


}

void MyGLWidget::paintGL(){
    QHash<int, entitySprite>::iterator s_i;
    entitySprite curSprite;
    QVector4D col[2];
    QVector4D hilight;
    float selectBox[2][2];
    int curIndex;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    col[1] = QVector4D(0.6, 0.6, 0.6, 1.0);
    col[0] = QVector4D(0.3, 0.7, 1.0, 1.0);

    screen.setToIdentity();
    screen.ortho(0, _width, _height, 0, -1, 1);

    ortho = screen;

    spriteDraw->draw(backSprite, SpriteMachine::BoundingBox(0, 0, _width, _height));
    spriteDraw->sync();

    ortho.translate(-camera.pos + QVector3D(getDrawWidth() * 0.5, getDrawHeight() * 0.5, 0));

    camera.original_pos = camera.pos;

    getCameraBounds();
    getMouseCurrent();

    drawSnapGuides();

    //-----------------------------------------------------------------------

    if(!op.disableGroupSelect && (op.operation == GROUP_SELECT)){
        doSelectBox(selectBox);
    }
    else if(op.operation == PAN){
        camera.pos = camera.pos - QVector3D(mouse.current.x() - mouse.start.x(), mouse.current.y() - mouse.start.y(), 0.0);
        getMouseCurrent();
        mouse.start = mouse.current;
    }

    if(op.isTransforming){
        modifyCurrentSelection(QVector2D(0.0,0.0), op.transform, op.transform_point);
        scrollOnExitBounds();
    }

    processSelection();
    for(curIndex = 0; curIndex < drawOrder.size(); curIndex++){
        curSprite = sprite[drawOrder[curIndex]];
        if(curSprite.isSelected){
            hilight = QVector4D(0.1,0.1,0.4,0.0);
        }
        else{
            hilight = QVector4D(0.0,0.0,0.0,0.0);
        }

        if(curSprite.isCovering){
            hilight.setW(qMax(hilight.w() - COVERING_TRANSPARENCY, -1.0));
        }
        spriteDraw->setDrawHeight(curSprite.sprite, curSprite.height);
        spriteDraw->setDrawWidth(curSprite.sprite, curSprite.width);
        spriteDraw->draw(curSprite.sprite, SpriteMachine::BoundingBox(curSprite.pos.x(), curSprite.pos.y(), 0, 0), curSprite.angle, curSprite.ID, hilight);
    }

    spriteDraw->sync(true);
    textDraw->sync();
    lineDraw->sync();
}


void MyGLWidget::resizeGL(int w, int h){
    this->_width = w;
    this->_height = h;
    glViewport(0, 0, w, h);
}

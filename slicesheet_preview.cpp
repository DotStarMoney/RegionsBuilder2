#include "slicesheet_preview.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include "autoslicer.h"

#define NONE_IMAGE 0
#define WAIT_IMAGE 1
#define REAL_IMAGE 2

sliceSheet_preview::sliceSheet_preview(QWidget *parent) :
    QWidget(parent)
{
    backImg = QImage(":/gfx/gfx/sliceBack.png");
    loadImg = QImage(":/gfx/gfx/sliceLoad.png");
    currentImage = backImg;
    outputImg = currentImage;
    isDepressed = false;
    curX = 0;
    curY = 0;
    curZoom = 1;
    dispImage = NONE_IMAGE;
    drawContours = true;
    drawImage = true;
    edgeA = 0.5;
    connect(&asyncLoadImage_watch, SIGNAL(finished()), this, SLOT(end_asyncLoadImage()));
    connect(&asyncScaleImage_watch, SIGNAL(finished()), this, SLOT(end_asyncScaleImage()));
}

int sliceSheet_preview::getImageMode(){
    return dispImage;
}


void sliceSheet_preview::setNewImage(QString filename){
    dispImage = WAIT_IMAGE;
    curX = 0;
    curY = 0;
    curZoom = 1;
    update();
    asyncLoadImage_future = QtConcurrent::run(this, &sliceSheet_preview::asyncLoadImage, filename);
    asyncLoadImage_watch.setFuture(asyncLoadImage_future);
}

void sliceSheet_preview::asyncLoadImage(QString filename){
    currentImage = QImage(filename);
    outputImg = currentImage;
}


void::sliceSheet_preview::end_asyncScaleImage(){
    dispImage = REAL_IMAGE;
    update();
    endWaiting();
}

void sliceSheet_preview::scaleImage(){
    if(dispImage == REAL_IMAGE){
       dispImage = WAIT_IMAGE;
       update();
       asyncScaleImage_future = QtConcurrent::run(this, &sliceSheet_preview::asyncScaleImage);
       asyncScaleImage_watch.setFuture(asyncScaleImage_future);
    }
}

void sliceSheet_preview::asyncScaleImage(){
    outputImg = currentImage.scaled(QSize(currentImage.width() / curZoom, currentImage.height() / curZoom), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void  sliceSheet_preview::end_asyncLoadImage(){
    dispImage = REAL_IMAGE;
    update();
    endWaiting();
}


QImage* sliceSheet_preview::fullImgPtr(){
    return &currentImage;
}


int sliceSheet_preview::getCurHeight(){
    return outputImg.height();
}

int sliceSheet_preview::getCurWidth(){
    return outputImg.width();
}


void sliceSheet_preview::mousePressEvent(QMouseEvent *e){
    isDepressed = true;
    oldPos = e->pos();
}
void sliceSheet_preview::mouseReleaseEvent(QMouseEvent *e){
    isDepressed = false;
}

float sliceSheet_preview::getZoom(){
    return curZoom;
}
float sliceSheet_preview::getX(){
    return curX;
}
float sliceSheet_preview::getY(){
    return curY;
}

void sliceSheet_preview::setDrawMode(bool im, bool ct){
    drawImage = im;
    drawContours = ct;
    update();
}

void sliceSheet_preview::setEdgeAlpga(float a){
    edgeA = a;
    update();
}

void sliceSheet_preview::mouseMoveEvent(QMouseEvent *e){
    QPoint delta;
    if(isDepressed){
        delta = -(e->pos() - oldPos);
        setParams(curX + delta.x(), curY + delta.y(), curZoom);
        oldPos = e->pos();
        update();
    }
}


QImage sliceSheet_preview::createSubImage(QImage *image, const QRect &rect){
    size_t offset = rect.x() * image->depth() / 8 + rect.y() * image->bytesPerLine();
    return QImage(image->bits() + offset, rect.width(), rect.height(), image->bytesPerLine(), image->format());
}

void sliceSheet_preview::setParams(float x, float y, float z){
    QSize newSize;
    newSize.setWidth(currentImage.width() / z);
    newSize.setHeight(currentImage.height() / z);
    if(newSize.width() < this->width()){
        endWaiting();
        return;
    }
    if(newSize.height() < this->height()){
        endWaiting();
        return;
    }
    if((x - this->width() * 0.5) < (-newSize.width() * 0.5)){
        x = -newSize.width() * 0.5 + this->width() * 0.5;
    }
    else if((x + this->width() * 0.5) >= (newSize.width() * 0.5)){
        x = newSize.width() * 0.5 - this->width() * 0.5;
    }
    if((y - this->height() * 0.5) < (-newSize.height() * 0.5)){
        y = -newSize.height() * 0.5 + this->height() * 0.5;
    }
    else if((y + this->height() * 0.5) >= (newSize.height() * 0.5)){
        y = newSize.height() * 0.5 - this->height() * 0.5;
    }
    if(curZoom != z){
        curZoom = z;
        scaleImage();
    }
    curX = x;
    curY = y;
}



void sliceSheet_preview::paintEvent(QPaintEvent *e){
    QPainter p;
    QImage out;
    QRect rect;

    p.begin(this);
    switch(dispImage){
    case NONE_IMAGE:

        p.drawImage(0, 0, backImg);

        break;
    case WAIT_IMAGE:

        p.drawImage(0, 0, loadImg);

        break;
    case REAL_IMAGE:

        rect.setLeft((curX + -(this->width() * 0.5)) + getCurWidth() * 0.5);
        rect.setTop((curY + -(this->height() * 0.5)) + getCurHeight() * 0.5);
        rect.setRight((curX + (this->width() * 0.5)) + getCurWidth() * 0.5);
        rect.setBottom((curY + (this->height() * 0.5)) + getCurHeight() * 0.5);

        if(rect.bottom() >= outputImg.height()) rect.setBottom(outputImg.height() - 1);
        if(rect.top() < 0) rect.setTop(0);
        if(rect.right() >= outputImg.width()) rect.setRight(outputImg.width() - 1);
        if(rect.left() < 0) rect.setLeft(0);

        out = createSubImage(&outputImg, rect);
        out = out.copy();
        if(drawContours || drawImage){
            p.fillRect(0, 0, this->width(), this->height(), QColor(27, 26, 27));

            if(drawImage){
                Autoslicer::drawContours(out, drawContours, edgeA);
            }

            p.drawImage(0, 0, out);
        }
        else{
            p.drawImage(0, 0, backImg);
        }

    }
    p.end();
}

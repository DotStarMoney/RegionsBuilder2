#ifndef SLICESHEET_PREVIEW_H
#define SLICESHEET_PREVIEW_H

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

class sliceSheet_preview : public QWidget
{
    Q_OBJECT
public:
    explicit sliceSheet_preview(QWidget *parent = 0);
    static QImage createSubImage(QImage* image, const QRect & rect);

    void setNewImage(QString filename);
    void scaleImage();

    void setParams(float x, float y, float z);
    int getCurWidth();
    int getCurHeight();
    float getZoom();
    float getX();
    float getY();

    int getImageMode();
    void setDrawMode(bool im, bool ct);
    void setEdgeAlpga(float a);
    QImage* fullImgPtr();
protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
private:
    QImage backImg;
    QImage loadImg;
    QImage currentImage;
    QImage outputImg;
    float curX;
    float curY;
    float curZoom;
    float edgeA;
    bool imageDraw;
    bool ContourDraw;
    bool isDepressed;
    QPoint oldPos;
    int dispImage;
    bool drawContours;
    bool drawImage;

    void asyncLoadImage(QString filename);
    QFutureWatcher<void> asyncLoadImage_watch;
    QFuture<void> asyncLoadImage_future;

    void asyncScaleImage();
    QFutureWatcher<void> asyncScaleImage_watch;
    QFuture<void> asyncScaleImage_future;


private slots:
    void end_asyncScaleImage();
    void end_asyncLoadImage();

signals:
    void endWaiting();
};

#endif // SLICESHEET_PREVIEW_H

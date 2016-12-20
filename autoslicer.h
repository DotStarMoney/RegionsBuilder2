#ifndef AUTOSLICER_H
#define AUTOSLICER_H

#include <QImage>
#include <opencv2/core/core.hpp>
#include <QString>
#include <QProgressBar>
#include <vector>

struct Autoslicer_sliceData{
public:
    Autoslicer_sliceData(cv::Rect r){
        bound = r;
    }
    cv::Rect bound;
    cv::Mat  component;
};

struct Autoslicer_rect{
public:
    Autoslicer_rect(int _x, int _y,
                    int _width, int _height,
                    int _r, int _index){
        x = _x;
        y = _y;
        width = _width;
        height = _height;
        r = _r;
        index = _index;
    }
    Autoslicer_rect(){}
    int x;
    int y;
    int width;
    int height;
    int r;
    int index;
};

class Autoslicer : public QObject
{
    Q_OBJECT
public:
    Autoslicer(QObject *parent = 0);
    static void drawContours(QImage& img, bool keepImg, float alphaThresh);

    bool buildPalette(QImage &img, QString palName,
                      float edgeAlpha, float scale, bool avoidRepack, bool premultiplyAlpha, int padding);
    void reset();
private:
    static QImage Mat2QImage(const cv::Mat& src);
    static cv::Mat QImage2Mat(const QImage& src);
    static bool minimalElement(const std::vector<cv::Point> &s);
    static long int repack(const std::vector<Autoslicer_sliceData> &s,
                       std::vector<Autoslicer_rect> &rects,
                       int bw, int bh, bool shouldRotate, int padding);
    int progress;
signals:
    void reportProgress(int);
};

#endif // AUTOSLICER_H

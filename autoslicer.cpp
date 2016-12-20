#include "autoslicer.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <QDebug>
#include <QFile>
#include <QDataStream>

#define MINIMAL_ELEMENT 10
#define MAX_DIMENSION 4096

Autoslicer::Autoslicer(QObject* parent) : QObject(parent)
{
    progress = 0;
}

long int Autoslicer::repack(const std::vector<Autoslicer_sliceData> &s,
                       std::vector<Autoslicer_rect> &rects,
                       int bw, int bh, bool shouldRotate, int padding){
    std::vector<Autoslicer_rect> freeRect;
    std::vector<Autoslicer_rect> remainRect;
    std::vector<Autoslicer_rect> BSSF;
    std::vector<Autoslicer_rect> BLSF;
    std::vector<Autoslicer_rect> BAF;
    std::vector<Autoslicer_rect> splits;

    Autoslicer_rect  cr;

    int              i;
    bool             shouldRemove;

    std::vector<int> kills;
    int              k1;
    int              k2;
    bool             shouldKill;
    int              ccount;

    int              BSSFc;
    int              BLSFc;
    int              BAFc;

    bool             recFound;
    bool             solFound;
    int              rnum;

    int              bestshorSF;
    int              bestlongSF;
    long int         bestareaF;

    int              cursel;
    int              currot;
    int              currec;

    int              curx;
    int              cury;
    int              curw;
    int              curh;
    int              curi;
    long int         curarea;

    int              leftHor;
    int              leftVer;
    int              shorSF;
    int              longSF;

    int              xbound;
    int              ybound;

    int              method;
    int              r;
    int              f;
    int              pe;
    int              ns;
    int              q;

    ccount = 99;
    BSSFc = -1;
    BLSFc = -1;
    BAFc = -1;
    recFound = false;
    solFound = false;
    rnum = 0;
    cursel = 0;
    currot = false;
    currec = 0;
    leftHor = 0;
    leftVer = 0;
    shorSF = 0;
    longSF = 0;

    for(method = 0; method < 3; method++){


        freeRect.clear();
        cr.width = bw;
        cr.height = bh;
        cr.x = 0;
        cr.y = 0;

        freeRect.push_back(cr);
        remainRect.clear();
        for(q = 0; q < s.size(); q++){
            remainRect.push_back(Autoslicer_rect(s[q].bound.x, s[q].bound.y,
                                                 s[q].bound.width + padding, s[q].bound.height + padding,
                                                 0, q));
        }

        rnum = remainRect.size();

        cursel = 0;
        currec = 0;
        currot = 0;
        curx = 0;
        cury = 0;
        curw = 0;
        curh = 0;
        curi = -1;

        xbound = 0;
        ybound = 0;

        while(rnum > 0){

            recFound = false;
            bestshorSF = qMax(bw, bh);
            bestlongSF = qMax(bw, bh);
            bestareaF = bw*bh;

            ccount++;

            for(r = 0; r < remainRect.size(); r++){

                for(f = 0; f < freeRect.size(); f++){
                    switch(method){
                    case 0:
                        if((freeRect[f].width >= remainRect[r].width) && (freeRect[f].height >= remainRect[r].height)){

                            leftHor = abs(freeRect[f].width - remainRect[r].width);
                            leftVer = abs(freeRect[f].height - remainRect[r].height);
                            shorSF = qMin(leftHor, leftVer);
                            longSF = qMax(leftHor, leftVer);

                            if((shorSF < bestshorSF) || ((shorSF == bestshorSF) && (longSF < bestlongSF))){
                                cursel = f;
                                currec = r;
                                currot = 0;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].width;
                                curh = remainRect[r].height;
                                curi = remainRect[r].index;
                                bestshorSF = shorSF;
                                bestlongSF = longSF;
                                recFound = true;
                            }
                        }

                        if(shouldRotate && (freeRect[f].width >= remainRect[r].height) && (freeRect[f].height >= remainRect[r].width)){

                            leftHor = abs(freeRect[f].width - remainRect[r].height);
                            leftVer = abs(freeRect[f].height - remainRect[r].width);
                            shorSF = qMin(leftHor, leftVer);
                            longSF = qMax(leftHor, leftVer);

                            if((shorSF < bestshorSF) || ((shorSF == bestshorSF) && (longSF < bestlongSF))){
                                cursel = f;
                                currec = r;
                                currot = 1;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].height;
                                curh = remainRect[r].width;
                                curi = remainRect[r].index;
                                bestshorSF = shorSF;
                                bestlongSF = longSF;
                                recFound = true;
                            }

                        }
                        break;
                    case 1:
                        if((freeRect[f].width >= remainRect[r].width) && (freeRect[f].height >= remainRect[r].height)){

                            leftHor = abs(freeRect[f].width - remainRect[r].width);
                            leftVer = abs(freeRect[f].height - remainRect[r].height);
                            shorSF = qMin(leftHor, leftVer);
                            longSF = qMax(leftHor, leftVer);

                            if((shorSF < bestshorSF) || ((shorSF == bestshorSF) && (longSF < bestlongSF))){
                                cursel = f;
                                currec = r;
                                currot = 0;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].width;
                                curh = remainRect[r].height;
                                curi = remainRect[r].index;
                                bestshorSF = shorSF;
                                bestlongSF = longSF;
                                recFound = true;
                            }
                        }

                        if(shouldRotate && (freeRect[f].width >= remainRect[r].height) && (freeRect[f].height >= remainRect[r].width)){

                            leftHor = abs(freeRect[f].width - remainRect[r].height);
                            leftVer = abs(freeRect[f].height - remainRect[r].width);
                            shorSF = qMin(leftHor, leftVer);
                            longSF = qMax(leftHor, leftVer);

                            if((shorSF < bestshorSF) || ((shorSF == bestshorSF) && (longSF < bestlongSF))){
                                cursel = f;
                                currec = r;
                                currot = 1;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].height;
                                curh = remainRect[r].width;
                                curi = remainRect[r].index;
                                bestshorSF = shorSF;
                                bestlongSF = longSF;
                                recFound = true;
                            }

                        }

                        break;
                    case 2:

                        curarea = freeRect[f].width*freeRect[f].height - curw*curh;
                        if((freeRect[f].width >= remainRect[r].width) && (freeRect[f].height >= remainRect[r].height)){

                            leftHor = abs(freeRect[f].width - remainRect[r].height);
                            leftVer = abs(freeRect[f].height - remainRect[r].width);
                            shorSF = qMin(leftHor, leftVer);

                            if((curarea < bestareaF) || ((curarea == bestareaF) && (shorSF < bestshorSF))){

                                cursel = f;
                                currec = r;
                                currot = 0;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].width;
                                curh = remainRect[r].height;
                                curi = remainRect[r].index;
                                bestareaF = curarea;
                                bestshorSF = shorSF;
                                recFound = true;
                            }
                        }
                        if(shouldRotate && (freeRect[f].width >= remainRect[r].height) && (freeRect[f].height >= remainRect[r].width)){

                            leftHor = abs(freeRect[f].width - remainRect[r].height);
                            leftVer = abs(freeRect[f].height - remainRect[r].width);
                            shorSF = qMin(leftHor, leftVer);

                            if((curarea < bestareaF) || ((curarea == bestareaF) && (shorSF < bestshorSF))){

                                cursel = f;
                                currec = r;
                                currot = 1;
                                curx = freeRect[f].x;
                                cury = freeRect[f].y;
                                curw = remainRect[r].height;
                                curh = remainRect[r].width;
                                curi = remainRect[r].index;
                                bestareaF = curarea;
                                bestshorSF = shorSF;
                                recFound = true;
                            }

                        }
                        break;
                    }
                }
            }



            if((curx + curw) > xbound) xbound = curx + curw;
            if((cury + curh) > ybound) ybound = cury + curh;

            if(!recFound){
                break;
            }
            else{
                remainRect.erase(remainRect.begin() + currec);
                rnum = remainRect.size();

                freeRect[cursel].r = ccount;
                splits.clear();
                i = 0;
                shouldKill = false;

                while(i < freeRect.size()){
                    if((curx > (freeRect[i].x + freeRect[i].width)) || ((curx + curw) < freeRect[i].x) ||
                       (cury > (freeRect[i].y + freeRect[i].height)) || ((cury + curh) < freeRect[i].y)){
                        i++;
                    }
                    else{
                        shouldRemove = false;
                        if((curx < (freeRect[i].x + freeRect[i].width)) && ((curx + curw) > freeRect[i].x)){
                            if((cury > freeRect[i].y) && (cury < (freeRect[i].y + freeRect[i].height))){
                                cr.height = cury - freeRect[i].y;
                                cr.width = freeRect[i].width;
                                cr.x = freeRect[i].x;
                                cr.y = freeRect[i].y;
                                splits.push_back(cr);
                                shouldRemove = true;
                                shouldKill = true;
                            }
                            if((cury + curh) < (freeRect[i].y + freeRect[i].height)){
                                cr.height = freeRect[i].y + freeRect[i].height - (cury+curh);
                                cr.width = freeRect[i].width;
                                cr.x = freeRect[i].x;
                                cr.y = cury+curh;
                                splits.push_back(cr);
                                shouldRemove = true;
                                shouldKill = true;
                            }
                        }

                        if((cury < (freeRect[i].y + freeRect[i].height)) && ((cury + curh) > freeRect[i].y)){
                            if((curx > freeRect[i].x) && (curx < (freeRect[i].x + freeRect[i].width))){
                                cr.width = curx - freeRect[i].x;
                                cr.height = freeRect[i].height;
                                cr.x = freeRect[i].x;
                                cr.y = freeRect[i].y;
                                splits.push_back(cr);
                                shouldRemove = true;
                                shouldKill = true;
                            }
                            if((curx + curw) < (freeRect[i].x + freeRect[i].width)){
                                cr.width = freeRect[i].x + freeRect[i].width - (curx + curw);
                                cr.height = freeRect[i].height;
                                cr.x = curx + curw;
                                cr.y = freeRect[i].y;
                                splits.push_back(cr);
                                shouldRemove = true;
                                shouldKill = true;
                            }
                        }

                        if(shouldRemove){
                            freeRect.erase(freeRect.begin() + i);
                        }
                        else{
                            i++;
                        }

                    }
                }

                cursel = -1;
                for(pe = 0; pe < freeRect.size(); pe++){
                    if(freeRect[pe].r == ccount) cursel = pe;
                }
                if(cursel > -1) freeRect.erase(freeRect.begin() + cursel);

                cr.x = curx;
                cr.y = cury;
                cr.width = curw;
                cr.height = curh;
                cr.r = currot;
                cr.index = curi;

                switch(method){
                case 0:
                    BSSF.push_back(cr);
                    break;
                case 1:
                    BLSF.push_back(cr);
                    break;
                case 2:
                    BAF.push_back(cr);
                    break;
                }

                for(ns = 0; ns < splits.size(); ns++){
                    freeRect.push_back(splits[ns]);
                }

                shouldRemove = true;
                k1 = 0;

                while(shouldRemove){
                    shouldRemove = false;
                    kills.clear();

                    if(freeRect.size() > 1){
                        while(k1 < (freeRect.size() - 1)){
                            shouldKill = false;
                            k2 = k1 + 1;
                            while(k2 < freeRect.size()){
                                if((freeRect[k2].x >= freeRect[k1].x) &&
                                   (freeRect[k2].y >= freeRect[k1].y) &&
                                   ((freeRect[k2].x + freeRect[k2].width) <= (freeRect[k1].x + freeRect[k1].width)) &&
                                   ((freeRect[k2].y + freeRect[k2].height) <= (freeRect[k1].y + freeRect[k1].height))){

                                    freeRect.erase(freeRect.begin() + k2);
                                    shouldRemove = true;

                                }
                                else{
                                    if((freeRect[k1].x >= freeRect[k2].x) &&
                                       (freeRect[k1].y >= freeRect[k2].y) &&
                                       ((freeRect[k1].x + freeRect[k1].width) <= (freeRect[k2].x + freeRect[k2].width)) &&
                                       ((freeRect[k1].y + freeRect[k1].height) <= (freeRect[k2].y + freeRect[k2].height))){

                                        shouldKill = true;
                                    }

                                    k2++;

                                }
                            }

                            if(shouldKill){
                                freeRect.erase(freeRect.begin() + k1);
                                shouldRemove = true;
                            }
                            else{
                                k1++;
                            }
                        }
                    }
                }
            }
        }

        if(recFound){
            solFound = true;
            switch(method){
            case 0:
                BSSFc = bw*bh - xbound*ybound;
                break;
            case 1:
                BLSFc = bw*bh - xbound*ybound;
                break;
            case 2:
                BAFc = bw*bh - xbound*ybound;
                break;
            }
        }
    }

    if(!solFound){
        return -1;
    }
    else{
        if((BSSFc >= BLSFc) && (BSSFc >= BAFc)){
            rects = BSSF;
            return BSSFc;
        }
        else if((BLSFc >= BSSFc) && (BLSFc >= BAFc)){
            rects = BLSF;
            return BLSFc;
        }
        else if((BAFc >= BSSFc) && (BAFc >= BLSFc)){
            rects = BAF;
            return BAFc;

        }
    }
}

bool Autoslicer::minimalElement(const std::vector<cv::Point> &s){
    if(s.size() <= MINIMAL_ELEMENT) return true;
    return false;
}

void Autoslicer::reset(){
    progress = 0;
}

bool Autoslicer::buildPalette(QImage &img, QString palName,
                              float edgeAlpha, float scale,
                              bool avoidRepack, bool premultiplyAlpha, int padding){
    cv::Mat original;
    cv::Mat contourMask;
    cv::Mat finalAtlas;
    unsigned int*  srcData_uint;
    unsigned char* dstData_uchar;
    QDataStream palFile;
    QFile       datFile;
    int alpha;
    int u, v;
    int intThresh;
    int i, j;
    int curDim;
    unsigned int outCol;
    float col[4];
    bool mustRepack;
    QImage outputImage;
    cv::Rect iRect;
    cv::Rect jRect;
    cv::Rect newB;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<Autoslicer_sliceData>   sliceData;
    std::vector<Autoslicer_rect>        newBounds;

    original = QImage2Mat(img);
    contourMask = cv::Mat::zeros(original.rows, original.cols, CV_8UC1);
    intThresh = (edgeAlpha * 255.0);

    reportProgress(10);

    for(v = 0; v < original.rows; v++){
        srcData_uint = original.ptr<unsigned int>(v);
        dstData_uchar = contourMask.ptr<unsigned char>(v);
        for(u = 0; u < original.cols; u++){
            alpha = (srcData_uint[u] & 0xff000000) >> 24;
            if(alpha < intThresh){
                dstData_uchar[u] = 0x00;
            }
            else{
                dstData_uchar[u] = 0xff;
            }
        }
    }

    reportProgress(20);

    cv::findContours(contourMask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    contours.erase(std::remove_if(contours.begin(), contours.end(), minimalElement), contours.end());

    reportProgress(30);

    for(i = 0; i < contours.size(); i++) sliceData.push_back(Autoslicer_sliceData(cv::boundingRect(contours[i])));

    if(avoidRepack){
        mustRepack = false;
        for(i = 0; i < contours.size() - 1; i++){
            iRect = sliceData[i].bound;
            for(j = i + 1; j < contours.size(); j++){
                jRect = sliceData[j].bound;
                if(!(((iRect.x + iRect.width) < jRect.x) ||
                     ((iRect.y + iRect.height) < jRect.y) ||
                     (iRect.x > (jRect.x + jRect.width)) ||
                     (iRect.y > (jRect.y + jRect.height)))){
                    mustRepack = true;
                    break;
                }
            }
            if(mustRepack) break;
        }
    }
    else{
        mustRepack = true;
    }
    reportProgress(40);

    if(mustRepack){
        for(i = 0; i < sliceData.size(); i++){
            contourMask = cv::Mat::zeros(sliceData[i].bound.height, sliceData[i].bound.width, CV_8UC4);
            cv::drawContours(contourMask, contours, i, cv::Scalar(1, 1, 1, 1), CV_FILLED,
                             8, cv::noArray(), 2147483647, cv::Point(-sliceData[i].bound.x, -sliceData[i].bound.y));
            cv::multiply(original(sliceData[i].bound), contourMask, sliceData[i].component);

            if(scale != 1.0){
                sliceData[i].bound.x *= scale;
                sliceData[i].bound.y *= scale;
                sliceData[i].bound.width *= scale;
                sliceData[i].bound.height *= scale;
                cv::resize(sliceData[i].component, sliceData[i].component, cv::Size(sliceData[i].bound.width, sliceData[i].bound.height));
            }
        }


        reportProgress(50);
        curDim = MAX_DIMENSION;
        while(repack(sliceData, newBounds, curDim, curDim, true, padding) > 0) curDim *= 0.5;
        curDim *= 2;

        finalAtlas = cv::Mat::zeros(curDim, curDim, CV_8UC4);
        for(i = 0; i < sliceData.size(); i++){
            j = newBounds[i].index;
            if(newBounds[i].r != 0) cv::transpose(sliceData[j].component, sliceData[j].component);
            newB = cv::Rect(newBounds[i].x, newBounds[i].y, newBounds[i].width - padding, newBounds[i].height - padding);
            sliceData[j].component.copyTo(finalAtlas(newB));
            sliceData[j].bound = newB;
        }
    }
    else{
        finalAtlas = original;
    }
    reportProgress(60);

    if(premultiplyAlpha){
        for(v = 0; v < finalAtlas.rows; v++){
            srcData_uint = finalAtlas.ptr<unsigned int>(v);
            for(u = 0; u < finalAtlas.cols; u++){
                col[0] = srcData_uint[u] & 0xff;
                col[1] = (srcData_uint[u] >> 8) & 0xff;
                col[2] = (srcData_uint[u] >> 16) & 0xff;
                col[3] = (srcData_uint[u] >> 24) & 0xff;
                col[0] = col[0] * (col[3] / 255.0);
                col[1] = col[1] * (col[3] / 255.0);
                col[2] = col[2] * (col[3] / 255.0);
                outCol = ((int) col[0]) | (((int) col[1]) << 8) | (((int) col[2]) << 16) | (((int) col[3]) << 24);
                srcData_uint[u] = outCol;
            }
        }
    }
    reportProgress(75);

    outputImage = Mat2QImage(finalAtlas);
    outputImage.save(palName + ".png", "PNG");

    reportProgress(90);

    datFile.setFileName(palName);
    datFile.open(QIODevice::WriteOnly);
    palFile.setDevice(&datFile);

    palFile << (qint32) sliceData.size();
    for(i = 0; i < sliceData.size(); i++){
        palFile << (qint32) sliceData[i].bound.x;
        palFile << (qint32) sliceData[i].bound.y;
        palFile << (qint32) sliceData[i].bound.width;
        palFile << (qint32) sliceData[i].bound.height;
    }
    datFile.close();

    reportProgress(100);
    return true;
}

void Autoslicer::drawContours(QImage& img, bool keepImg, float alphaThresh){
    cv::Mat working;
    cv::Mat convWorking;
    cv::Mat original;
    unsigned int intThresh;
    int r;
    int c;
    int u, v;
    float valM;
    unsigned int alpha;
    unsigned int* srcData;
    unsigned int* cpyData;
    uchar*        dstData;

    std::vector<std::vector<cv::Point>> contours;

    original = QImage2Mat(img);
    CV_Assert(original.depth() != sizeof(unsigned int));

    intThresh = (alphaThresh * 255.0);
    r = original.rows;
    c = original.cols;
    working = cv::Mat(r, c, CV_8UC1, 255);

    for(u = 0; u < r; u++){
        srcData = original.ptr<unsigned int>(u);
        dstData = working.ptr<uchar>(u);
        for(v = 0; v < c; v++){
            alpha = (srcData[v] & 0xff000000) >> 24;
            if(alpha < intThresh){
                dstData[v] = 0x00;
            }
            else{
                dstData[v] = 0xff;
            }
        }
    }
    cv::Canny(working, working, 0.5, 1.5);
    cv::GaussianBlur(working, working, cv::Size(5,5), 1);
    cv::cvtColor(working, working, CV_GRAY2BGRA);

    convWorking = cv::Mat(working.rows, working.cols, CV_8UC4);
    r = working.rows;
    c = working.cols;
    for(u = 0; u < r; u++){
        srcData = working.ptr<unsigned int>(u);
        cpyData = convWorking.ptr<unsigned int>(u);
        for(v = 0; v < c; v++){
            cpyData[v] = (srcData[v] & 0x000000ff) | ((int) (((srcData[v] & 0x0000ff00) >> 8) * 0.5) << 8) | ((srcData[v] & 0xff) << 24);
        }
    }
    if(!keepImg){
        valM = 0;
    }
    else{
        valM = 0.5;
    }
    cv::addWeighted(convWorking, 2, original, valM, 0, working);

    img = Mat2QImage(working);
}


QImage Autoslicer::Mat2QImage(const cv::Mat &src){
    cv::Mat temp;
    QImage destImp;
    QImage destRet;

    cv::cvtColor(src, temp, CV_BGRA2RGBA);

    destImp = QImage((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGBA8888);
    destRet = destImp;
    destRet.detach();

    return destRet;
}

cv::Mat Autoslicer::QImage2Mat(const QImage &src){
    cv::Mat temp;

    temp = cv::Mat(src.height(), src.width(), CV_8UC4, (uchar*)src.bits(), src.bytesPerLine());

    return temp;
}

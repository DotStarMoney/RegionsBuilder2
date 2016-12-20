#include "fontface.h"
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

Fontface::Fontface(QString filename)
{
    face = "";
    loadFont(filename);
}

Fontface::Fontface(){
    //
    face = "";
}

Fontface::~Fontface(){
    QHash<int, QHash<int, int>*>::iterator i;
    QHash<int, int>* kern;
    if(kernings.size() > 0){
        qDebug() << kernings.size();
        for(i = kernings.begin(); i != kernings.end(); i++){
            kern = *i;
            delete kern;
        }
    }
}

void Fontface::loadFont(QString filename){
    QFile       fontFile;
    QTextStream textStream;
    QString     textline;
    QStringList tokens;
    QString     field;
    QString     section;
    QString     value;
    int         curVal;
    int         kerningData[3];
    Character   curChar;
    QHash<int, int>* kernHash;

    fontFile.setFileName(filename + ".fnt");
    if(!fontFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        error("could not open file");
        return;
    }

    textStream.setDevice(&fontFile);
    while (!textStream.atEnd()){
        textline = textStream.readLine();
        tokens = textline.split(QRegExp("( +)|(=+)"));
        section = tokens.front();
        tokens.pop_front();
        while(tokens.size() > 0){
            field = tokens.front();
            tokens.pop_front();
            value = tokens.front();

            if(section == "info"){
                if(field == "face"){
                    face = value.mid(1, value.size() - 2);
                }
                else if(field == "size"){
                    ptSize = value.toInt();
                }
                else if(field == "padding"){
                    padding[0] = value.section(",", 0).toInt();
                    padding[1] = value.section(",", 1).toInt();
                    padding[2] = value.section(",", 2).toInt();
                    padding[3] = value.section(",", 3).toInt();
                }
                else if(field == "spacing"){
                    spacing[0] = value.section(",", 0).toInt();
                    spacing[1] = value.section(",", 1).toInt();
                }
            }
            else if(section == "common"){
                if(field == "lineHeight"){
                    lineHeight = value.toInt();
                }
                else if(field == "base"){
                    baseHeight = value.toInt();
                }
                else if(field == "scaleW"){
                    scaleW = value.toInt();
                }
                else if(field == "scaleH"){
                    scaleH = value.toInt();
                }
                else if(field == "pages"){
                    numPages = value.toInt();
                    if(numPages > 1){
                        error("only one page per font supported");
                        return;
                    }
                }
            }
            else if(section == "page"){
                if(field == "id"){
                    curVal = value.toInt();
                }
                else if(field == "file"){
                    pageFileName = filename + ".png"; //value.mid(1, value.size() - 2);
                    pageImage = QImage(pageFileName);
                }
            }
            else if(section == "chars"){
                if(field == "count"){
                    numChars = value.toInt();
                }
            }
            else if(section == "char"){
                if(field == "id"){
                    curVal = value.toInt();
                }
                else if(field == "x"){
                    curChar.xPos = value.toInt();
                }
                else if(field == "y"){
                    curChar.yPos = value.toInt();
                }
                else if(field == "width"){
                    curChar.width = value.toInt();
                }
                else if(field == "height"){
                    curChar.height = value.toInt();
                }
                else if(field == "xoffset"){
                    curChar.xOffset = value.toInt();
                }
                else if(field == "yoffset"){
                    curChar.yOffset = value.toInt();
                }
                else if(field == "xadvance"){
                    curChar.xadvance = value.toInt();
                }
                else if(field == "page"){
                    curChar.page = value.toInt();
                }
                else if(field == "chnl"){
                    curChar.channel = value.toInt();
                }
            }
            else if(section == "kernings"){
                if(field == "count"){
                    numKernings = value.toInt();
                }
            }
            else if(section == "kerning"){
                if(field == "first"){
                    kerningData[0] = value.toInt();
                }
                if(field == "second"){
                    kerningData[1] = value.toInt();
                }
                if(field == "amount"){
                    kerningData[2] = value.toInt();
                }
            }

            if(tokens.size() < 3) break;
            tokens.pop_front();
        }
        if(section == "char"){
            chars.insert(curVal, curChar);
        }
        else if(section == "kerning"){
            if(kernings.contains(kerningData[0])){
                kernHash = kernings.value(kerningData[0]);
                kernHash->insert(kerningData[1], kerningData[2]);
            }
            else{
                kernHash = new QHash<int, int>();
                kernHash->insert(kerningData[1], kerningData[2]);
                kernings.insert(kerningData[0], kernHash);
            }
        }
    }

}

void Fontface::computeTextGeometry(QString text, double size,
                                   QVector2D *&pos, QVector2D *&tex,
                                   int *&chnl,
                                   float &width, float &height){
    int       i;
    int       oldChar;
    int       curChar;
    int       curXPos;
    int       iOff;
    float     lowest;
    float     rightest;
    float     invTexH;
    float     invTexW;
    Character cur;
    QHash<int, int>* kern;

    pos = new QVector2D[text.length() * 4];
    tex = new QVector2D[text.length() * 4];
    chnl = new int[text.length()];

    curXPos = 0;
    oldChar = 0;
    curChar = 0;
    iOff = 0;
    lowest = 0;
    rightest = 0;

    invTexH = 1.0 / (float) pageImage.height();
    invTexW = 1.0 / (float) pageImage.width();

    for(i = 0; i < text.length(); i++){
        oldChar = curChar;
        curChar = text.at(i).unicode();
        if(!chars.contains(curChar)) curChar = 63;

        if(i > 0){
            if(kernings.contains(oldChar)){
                kern = kernings.value(oldChar);
                if(kern->contains(curChar)){
                    curXPos += kern->value(curChar) * size;
                }
            }
        }

        cur = chars.value(curChar);

        pos[iOff + 0] = QVector2D(curXPos + cur.xOffset * size, cur.yOffset * size);
        pos[iOff + 1] = QVector2D(pos[iOff + 0].x(), pos[iOff + 0].y() + cur.height * size);
        pos[iOff + 2] = QVector2D(pos[iOff + 0].x() + cur.width * size, pos[iOff + 1].y());
        pos[iOff + 3] = QVector2D(pos[iOff + 2].x(), pos[iOff + 0].y());

        if(pos[iOff + 2].x() > rightest) rightest = pos[iOff + 2].x() + 1;
        if(pos[iOff + 2].y() > lowest)   lowest   = pos[iOff + 2].y() + 1;

        tex[iOff + 0] = QVector2D(cur.xPos, cur.yPos);
        tex[iOff + 1] = QVector2D(tex[iOff + 0].x(), tex[iOff + 0].y() + cur.height);
        tex[iOff + 2] = QVector2D(tex[iOff + 0].x() + cur.width, tex[iOff + 1].y());
        tex[iOff + 3] = QVector2D(tex[iOff + 2].x(), tex[iOff + 0].y());

        tex[iOff + 0] = QVector2D(tex[iOff + 0].x() * invTexW, tex[iOff + 0].y() * invTexH);
        tex[iOff + 1] = QVector2D(tex[iOff + 1].x() * invTexW, tex[iOff + 1].y() * invTexH);
        tex[iOff + 2] = QVector2D(tex[iOff + 2].x() * invTexW, tex[iOff + 2].y() * invTexH);
        tex[iOff + 3] = QVector2D(tex[iOff + 3].x() * invTexW, tex[iOff + 3].y() * invTexH);

        chnl[i] = cur.channel;

        curXPos += cur.xadvance * size;

        iOff += 4;
    }
    width = rightest;
    height = lowest;
}


QImage& Fontface::getPage(){
    if(face == "") warning("no font has been loaded.");
    return pageImage;
}

QString Fontface::getFace(){
    if(face == "") warning("no font has been loaded.");
    return face;
}

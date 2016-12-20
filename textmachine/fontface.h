#ifndef FONTFACE_H
#define FONTFACE_H

#include <QString>
#include <QVector>
#include <QHash>
#include <QImage>
#include "character.h"
#include "../errlog.h"
#include <QVector2D>

class Character;

class Fontface : public ErrLog::Log
{
public:
    Fontface(QString filename);
    Fontface();
    ~Fontface();
    void loadFont(QString filename);
    QString getFace();
    QImage& getPage();
    void computeTextGeometry(QString text, double size,
                             QVector2D* &pos, QVector2D* &tex,
                             int* &chnl,
                             float &width, float &height);

private:
    QString          face;
    int              ptSize;
    int              padding[4];
    int              spacing[2];

    int              lineHeight;
    int              baseHeight;

    int              scaleW;
    int              scaleH;
    int              numPages;
    QString          pageFileName;
    QImage           pageImage;

    int                   numChars;
    QHash<int, Character> chars;

    int                         numKernings;
    QHash<int, QHash<int, int>*> kernings;

};

#endif // FONTFACE_H

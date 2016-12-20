#ifndef CHARACTER_H
#define CHARACTER_H

#include "fontface.h"

class Character
{
    friend class Fontface;
public:
    Character();
private:
    int id;
    int xPos;
    int yPos;
    int width;
    int height;
    int xOffset;
    int yOffset;
    int xadvance;
    int page;
    int channel;
};

#endif // CHARACTER_H

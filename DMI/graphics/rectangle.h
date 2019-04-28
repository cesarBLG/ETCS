#ifndef _RECTANGLE_H
#define _RECTANGLE_H
#include "graphic.h"
#include "color.h"
class rectangle : public graphic
{
    public:
    float sx;
    float sy;
    float x;
    float y;
    Color color;
    rectangle(float x, float y, float sx, float sy, Color col) : graphic(RECTANGLE), sx(sx), sy(sy), x(x), y(y), color(col){}
};
#endif
#ifndef _LINE_H
#define _LINE_H
#include "graphic.h"
#include "color.h"
class line : public graphic
{
    protected:
    line() : graphic(LINE){}
    public:
    float x1;
    float y1;
    float x2;
    float y2;
    Color color;
    line(float x1, float y1, float x2, float y2, Color col) : graphic(LINE), x1(x1), y1(y1), x2(x2), y2(y2), color(col){}
};
#endif
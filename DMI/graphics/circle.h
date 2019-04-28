#ifndef _CIRCLE_H
#define _CIRCLE_H
#include "graphic.h"
class circle : public graphic
{
    public:
    float cx;
    float cy;
    float radius;
    circle(float cx, float cy, float radius) : graphic(CIRCLE), cx(cx), cy(cy), radius(radius) {}
};
#endif
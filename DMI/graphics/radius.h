#ifndef _RADIUS_H
#define _RADIUS_H
#include "line.h"
#include <cmath>
class radius : public line
{
    public:
    radius(float angle, float cx, float cy, float rmin, float rmax, Color col) : line()
    {
        float c = cosf(angle);
        float s = sinf(angle);
        x1 = rmin*c + cx;
        x2 = rmax*c + cx;
        y1 = rmin*s + cy;
        y2 = rmax*s + cy;
        color = col;
    }
};
#endif
#include "graphics/color.h"
#include <cstdio>
#include "graphics/drawing.h"
using namespace std;
class Region
{
    float x;
    float y;
    float sx;
    float sy;
    public:
    Region(float x, float y, float sx, float sy)
    {
        this->x = x;
        this->y = y;
        this->sx = sx;
        this->sy = sy;
    }
    void flush()
    {
        return;
        /*setColor(DarkBlue);
        glBegin(GL_POLYGON);
        glVertex2f(x+sx/2,y+sy/2);
        glVertex2f(x+sx/2,y-sy/2);
        glVertex2f(x-sx/2,y-sy/2);
        glVertex2f(x-sx/2,y+sy/2);
        glEnd();*/
    }
    void displayText(const char *str, float size, Color c)
    {
        drawText(str, x, y, sx, sy, size, c, CENTER);
    }
    void displayImage(const char *name)
    {
        drawImage(name, x, y, sx, sy);
    }
};
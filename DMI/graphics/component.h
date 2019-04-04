#ifndef _COMPONENT_H
#define _COMPONENT_H
#include "color.h"
#include "SDL2/SDL.h"
#include "drawing.h"
#include <functional>
#include <string>
extern float offset[];
using namespace std;
class Component
{
    public:
    float x;
    float y;
    float sx;
    float sy;
    protected:
    inline int getX(float val)
    {
        return round(getScale(val+x) + offset[0]);
    }
    inline int getY(float val)
    {   
        return round(getScale(val+y) + offset[1]);
    }
    void getXpoints(float *source, short * dest, int n)
    {
        for(int i=0; i<n; i++)
        {
            dest[i] = (short)getX(source[i]);
        }
    }
    void getYpoints(float *source, short * dest, int n)
    {
        for(int i=0; i<n; i++)
        {
            dest[i] = (short)getY(source[i]);
        }
    }
    SDL_Surface *bgSurf;
    string surfloc = "";
    bool ack = false;
    function<void()> pressedAction;
    function<void()> display = nullptr;
    public:
    bool isButton = false;
    bool dispBorder = true;
    bool isSensitive(){return pressedAction != nullptr;}
    Component(){}
    Component(float sx, float sy, function<void()> display);
    ~Component();
    void setPressed(bool value);
    void setPressedAction(function<void()> action);
    void setAck(function<void()> ackAction);
    void setDisplayFunction(function<void()> display);
    void setSize(float sx, float sy);
    void setLocation(float x, float y);
    virtual void paint();
    void foo();
    void drawArc(float ang0, float ang1, float r, float cx, float cy);
    void rotateVertex(float *vx, float *vy, int pcount, float cx, float cy, float angle);
    void drawLine(float x1, float y1, float x2, float y2);
    void drawLine(float x1, float y1, float x2, float y2, Color c);
    void drawPolygon(float *x, float *y, int n);
    void drawBox(float sx, float sy, Color c);
    void drawCircle(float cx, float cy, float radius);
    void drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy);
    void drawRadius(float cx, float cy, float rmin, float rmax, float ang);
    void drawSurface(SDL_Surface *surf, float cx, float cy, float sx, float sy, bool destroy = true);
    void drawImage(const char *name, float cx, float cy, float sx, float sy);
    void drawText(const char *text, float cx, float cy, float sx, float sy, float size, Color col = White, int align = CENTER, int aspect = 0);
    void setBackgroundImage(const char *name);
    void setText(const char* text, float size, Color c);
    void setBorder(Color c);
};
extern Component Z;
extern Component C;
extern Component Y;
#endif
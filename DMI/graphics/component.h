#ifndef _COMPONENT_H
#define _COMPONENT_H
#include "color.h"
#include "SDL2/SDL.h"
#include "drawing.h"
#include "graphic.h"
#include "text_graphic.h"
#include "image_graphic.h"
#include <functional>
#include <string>
#include <cmath>
#include <vector>
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
    Color bgColor = DarkBlue;
    Color fgColor = White;
    vector<graphic*> graphics;
    bool ack = false;
    function<void()> pressedAction;
    function<void()> display = nullptr;
    public:
    void clear()
    {
        for(int i=0; i<graphics.size(); i++)
        {
            delete graphics[i];
        }
        graphics.clear();
    }
    bool isButton = false;
    bool dispBorder = true;
    bool isSensitive(){return pressedAction != nullptr;}
    float touch_up = 0;
    float touch_down = 0;
    float touch_left = 0;
    float touch_right = 0;
    Component(){}
    Component(float sx, float sy, function<void()> display = nullptr);
    virtual ~Component();
    void setPressed(bool value);
    void setPressedAction(function<void()> action);
    void setAck(function<void()> ackAction);
    void setDisplayFunction(function<void()> display);
    void setSize(float sx, float sy);
    void setLocation(float x, float y);
    virtual void paint();
    void drawArc(float ang0, float ang1, float r, float cx, float cy);
    void rotateVertex(float *vx, float *vy, int pcount, float cx, float cy, float angle);
    void draw(graphic *g, bool destroy = false);
    void drawLine(float x1, float y1, float x2, float y2);
    void drawLine(float x1, float y1, float x2, float y2, Color c);
    void drawPolygon(float *x, float *y, int n);
    void drawCircle(float radius, float cx, float cy);
    void drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy);
    void drawRadius(float cx, float cy, float rmin, float rmax, float ang);
    void drawRectangle(float x, float y, float w, float h, Color c, int align = LEFT | UP);
    void addRectangle(float x, float y, float w, float h, Color c, int align = LEFT | UP);
    void drawTexture(SDL_Texture *tex, float cx, float cy, float sx, float sy);
    void add(graphic *g) { graphics.push_back(g); }
    void drawText(string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    void addText(string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    text_graphic *getText(string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    void getTextGraphic(texture *t, string text, float x, float y, float size, Color col, int align, int aspect);
    void drawImage(string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void addImage(string path, float cx=0, float cy=0, float sx=0, float sy=0);
    image_graphic *getImage(string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void getImageGraphic(texture *t, string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void setBackgroundColor(Color c);
    void setForegroundColor(Color c);
    string text;
    float text_size;
    Color text_color;
    void setLabel(string text, float size, Color c);
    void setText(string text, float size, Color c);
    void setBorder(Color c);
};
extern Component Z;
extern Component C;
extern Component Y;
#endif
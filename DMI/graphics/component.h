/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    bool ack = false;
    std::function<void()> pressedAction;
    std::function<void()> display = nullptr;
    public:
    std::vector<graphic*> graphics;
    void clear();
    bool isButton = false;
    bool dispBorder = true;
    bool upType = true;
    bool delayType = false;
    int64_t firstPressedTime = 0;
    int64_t lastPressedTime = 0;
    bool isSensitive(){return pressedAction != nullptr;}
    float touch_up = 0;
    float touch_down = 0;
    float touch_left = 0;
    float touch_right = 0;
    Component(){}
    Component(float sx, float sy, std::function<void()> display = nullptr);
    virtual ~Component();
    void setPressed();
    void setPressedAction(std::function<void()> action);
    void setAck(std::function<void()> ackAction);
    void setDisplayFunction(std::function<void()> display);
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
    void drawText(std::string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    void addText(std::string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    text_graphic *getText(std::string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
    void getTextGraphic(texture *t, std::string text, float x, float y, float size, Color col, int align, int aspect);
    void drawImage(std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void addImage(std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
    image_graphic *getImage(std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void getImageGraphic(texture *t, std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
    void setBackgroundColor(Color c);
    void setForegroundColor(Color c);
    std::string text;
    float text_size;
    Color text_color;
    void setLabel(std::string text, float size, Color c);
    void setText(std::string text, float size, Color c);
    void addBorder(Color c);
    void setBorder(Color c);
};
extern Component Z;
extern Component C;
extern Component Y;
#endif

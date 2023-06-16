/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _COMPONENT_H
#define _COMPONENT_H
#include "color.h"
#include "drawing.h"
#include "graphic.h"
#include "text_graphic.h"
#include "image_graphic.h"
#include <functional>
#include <string>
#include <cmath>
#include <vector>
#include <set>
extern float offset[];
class Component
{
    public:
        float x;
        float y;
        float sx;
        float sy;
        static std::set<Component*> _instances;
        static void externalAckButton(int time);

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
        int flash_style = 0;
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
        void draw(graphic *g);
        void drawLine(float x1, float y1, float x2, float y2);
        void drawLine(float x1, float y1, float x2, float y2, Color c);
        void drawPolygon(float *x, float *y, int n);
        void drawCircle(float radius, float cx, float cy);
        void drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy);
        void drawRadius(float cx, float cy, float rmin, float rmax, float ang);
        void drawRectangle(float x, float y, float w, float h, Color c, int align = LEFT | UP);
        void addRectangle(float x, float y, float w, float h, Color c, int align = LEFT | UP);
        void drawTexture(std::shared_ptr<Platform::Image> tex, float cx, float cy, float sx, float sy);
        void add(graphic* g) { graphics.push_back(g); }
        void addText(std::string text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
        text_graphic *getText(const std::string &text, float x=0, float y=0, float size=12, Color col=White, int align=CENTER, int aspect=0);
        std::unique_ptr<text_graphic> getTextUnique(const std::string &text, float x = 0, float y = 0, float size = 12, Color col = White, int align = CENTER, int aspect = 0);
        static std::shared_ptr<Platform::Image> getTextGraphic(std::string text, float size, Color col, int aspect, int align=CENTER);
        void addImage(std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
        image_graphic *getImage(std::string path, float cx=0, float cy=0, float sx=0, float sy=0);
        static std::shared_ptr<Platform::Image> getImageGraphic(std::string path);
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

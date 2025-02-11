/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "component.h"
#include "display.h"
#include <algorithm>
#include <cmath>
#include "flash.h"
#include "../sound/sound.h"
#include "texture.h"
#include "rectangle.h"
#include "line.h"
#include "circle.h"
#include "button.h"
#include "platform_runtime.h"
using namespace std;

Component Z(640, 15, nullptr);
Component Y(640, 15, nullptr);
Component::Component(float sx, float sy, function<void()> display)
{
    this->sx = sx;
    this->sy = sy;
    this->display = display;
}
Component::~Component()
{
    clear();
}
void Component::clear()
{
    for(int i=0; i<graphics.size(); i++)
    {
        delete graphics[i];
    }
    graphics.clear();
}
void Component::setPressedAction(function<void()> action)
{
    pressedAction = action;
}
void Component::setAck(function<void()> ackAction)
{
    pressedAction = ackAction;
    ack = ackAction != nullptr;
}
void Component::setPressed()
{
    if (pressedAction != nullptr)
    {
        pressedAction();
    }
}
void Component::setDisplayFunction(function<void()> display)
{
    this->display = display;
}
void Component::setSize(float sx, float sy)
{
    this->sx = sx;
    this->sy = sy;
}
void Component::setLocation(float x, float y)
{
    this->x = x;
    this->y = y;
}
void Component::drawLine(float x1, float y1, float x2, float y2)
{
    platform->draw_line(getX(x1), getY(y1), getX(x2), getY(y2));
}
void Component::drawLine(float x1, float y1, float x2, float y2, Color c)
{
    platform->set_color(c);
    drawLine(x1, y1, x2, y2);
}
void Component::paint()
{
    bool show = true;
    if (flash_style != 0)
    {
        bool fast = flash_style & 1;
        bool counter = (flash_style & 2)>>1;
        show = (fast ? (flash_state&1) : ((flash_state>>1)&1)) == (counter ? 0 : 1);
    }
    if (show || !(flash_style & 4))
    {
        if(bgColor != DarkBlue) drawRectangle(0, 0, sx, sy, bgColor);
        for(int i=0; i<graphics.size(); i++)
        {
            draw(graphics[i]);
        }
        if(display != nullptr) display();
    }
    if ((show && flash_style != 0 && !(flash_style & 4)) || (ack && (flash_state & 2)))
    {
        drawRectangle(0, 0, 2, sy, Yellow);
        drawRectangle(sx - 2, 0, 2, sy, Yellow);
        drawRectangle(0, 0, sx, 2, Yellow);
        drawRectangle(0, sy - 2, sx, 2, Yellow);
    }
    else if(dispBorder)
    {
        drawRectangle(0, 0, 1, sy - 1, Black);
        drawRectangle(sx - 1, 0, 1, sy - 1, Shadow);
        drawRectangle(0, 0, sx - 1, 1, Black);
        drawRectangle(0, sy - 1, sx - 1, 1, Shadow);
    }
}
void Component::drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy)
{
    platform->draw_arc_filled(getX(cx), getY(cy), rmin, rmax, ang0, ang1);
}
void Component::rotateVertex(float *vx, float *vy, int pcount, float cx, float cy, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    for(int i = 0; i < pcount; i++)
    {
        float dx = vx[i];
        float dy = vy[i];
        vx[i] = cx - dy * c - dx * s;
        vy[i] = cy - dy * s + dx * c;
    }
}
void Component::draw(graphic *graph)
{
    if(graph == nullptr) return;
    switch(graph->type)
    {
        case TEXTURE:{
            texture *t = (texture*)graph;
            if(t->tex == nullptr) return;
            drawTexture(t->tex, t->x, t->y);
            break;}
        case RECTANGLE:{
            rectangle *r = (rectangle*)graph;
            drawRectangle(r->x, r->y, r->sx, r->sy, r->color);
            break;}
        case LINE:{
            line *l = (line*)graph;
            drawLine(l->x1, l->y1, l->x2, l->y2, l->color);
            break;}
        case CIRCLE:{
            circle *c = (circle*)graph;
            drawCircle(c->radius, c->cx, c->cy);
            break;}
        default:
            break;
    }
}
void Component::drawConvexPolygon(float* x, float* y, int n)
{
    std::vector<std::pair<float, float>> poly;
    poly.reserve(n);
    for (int i = 0; i < n; i++)
        poly.push_back(std::make_pair(getX(x[i]), getY(y[i])));
    platform->draw_convex_polygon_filled(poly);
}
void Component::drawCircle(float radius, float cx, float cy)
{
    platform->draw_circle_filled(getX(cx), getY(cy), radius);
}
void Component::addRectangle(float x, float y, float w, float h, Color c, int align)
{
    add(new rectangle(x,y,w,h,c));
}
void Component::drawRectangle(float x, float y, float w, float h, Color c, int align)
{
    platform->set_color(c);
    if(!(align & LEFT)) x = sx / 2 + x - w / 2;
    if(!(align & UP)) y = sy / 2 + y - h / 2;
    platform->draw_rect_filled(getX(x), getY(y), w, h);
}
void Component::drawRadius(float cx, float cy, float rmin, float rmax, float ang)
{
    float c = cosf(ang);
    float s = sinf(ang);
    drawLine(cx - rmin * c, cy - rmin * s, cx - rmax * c, cy - rmax * s);
}
void Component::drawTexture(std::shared_ptr<UiPlatform::Image> tex, float cx, float cy)
{
    auto size = tex->size();
    platform->draw_image(*tex, getX(cx - size.first / 2), getY(cy - size.second / 2));
}
void Component::addText(string text, float x, float y, float size, Color col, int align, int aspect, float width)
{
    if(text=="") return;
    add(getText(text, x, y, size, col, align, aspect, width));
}
text_graphic* Component::getText(const string &text, float x, float y, float size, Color col, int align, int aspect, float width)
{
    return getTextUnique(text, x, y, size, col, align, aspect, width).release();
}
std::unique_ptr<text_graphic> Component::getTextUnique(const string &text, float x, float y, float size, Color col, int align, int aspect, float width)
{
    std::unique_ptr<text_graphic> t = std::make_unique<text_graphic>();
    t->text = text;
    t->offx = x;
    t->offy = y;
    t->size = size;
    t->color = col;
    t->alignment = align;
    t->aspect = aspect;
    int v = text.find('\n');
    t->tex = getTextGraphic(text, size, col, aspect, align&(LEFT|RIGHT), width);
    float sx = t->tex == nullptr ? 0 : t->tex->size().first;
    float sy = t->tex == nullptr ? 0 : t->tex->size().second;
    if (align & UP) y = y + sy / 2;
    else if (align & DOWN) y = (this->sy - y) - sy / 2;
    else y = y + this->sy / 2;
    if (align & LEFT) x = x + sx / 2;
    else if (align & RIGHT) x = (this->sx - x) - sx / 2;
    else x = x + this->sx / 2;
    t->x = x;
    t->y = y;
    t->width = sx;
    t->height = sy;
    return t;
}
std::shared_ptr<UiPlatform::Image> Component::getTextGraphic(string text, float size, Color col, int aspect, int align, float width)
{
    auto font = platform->load_font(size, (aspect & 1) != 0, get_language());
    return platform->make_wrapped_text_image(text, *font, width, align, col);
}
void Component::addImage(string path, float cx, float cy, float sx, float sy)
{
    add(getImage(path, cx, cy, sx, sy));
}
image_graphic *Component::getImage(string path, float cx, float cy, float sx, float sy)
{
    image_graphic *ig = new image_graphic();
    ig->path = path;
    ig->tex = getImageGraphic(ig->path);
    if(sx > 0 && sy > 0)
    {
        ig->width = sx;
        ig->height = sy;
        ig->x = cx;
        ig->y = cy;
    }
    else
    {
        ig->width = ig->tex->size().first;
        ig->height = ig->tex->size().second;
        ig->x = this->sx/2;
        ig->y = this->sy/2;
    }
    return ig;
}
std::shared_ptr<UiPlatform::Image> Component::getImageGraphic(string path)
{
    return platform->load_image(path);
}
void Component::setBorder(Color c)
{
    platform->set_color(c);
    platform->draw_rect(getX(0), getY(0), getX(sx) - getX(0), getY(sy) - getY(0));
}
void Component::addBorder(Color c)
{
    addRectangle(0,0,1,sy,MediumGrey);
    addRectangle(sx-1,0,1,sy,MediumGrey);
    addRectangle(0,0,sx,1,MediumGrey);
    addRectangle(0,sy-1,sx,1,MediumGrey);
}
void Component::setBackgroundColor(Color c)
{
    bgColor = c;
}
void Component::setForegroundColor(Color c)
{
    fgColor = c;
}

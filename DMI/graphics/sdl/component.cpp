/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../component.h"
#include "../display.h"
#include "gfx_primitives.h"
#include <algorithm>
#include <cmath>
#include "../flash.h"
#include "../../sound/sound.h"
#include "../texture.h"
#include "../rectangle.h"
#include "../line.h"
#include "../circle.h"
#include "../button.h"
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
    int res = SDL_RenderDrawLine(sdlren, getX(x1), getY(y1), getX(x2), getY(y2));
    if(res < 0) printf("Failed to draw line. SDL Error: %s\n", SDL_GetError());
}
void Component::drawLine(float x1, float y1, float x2, float y2, Color c)
{
    setColor(c);
    drawLine(x1, y1, x2, y2);
}
void Component::paint()
{
    bool show = true;;
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
    if (show && flash_style != 0 && !(flash_style & 4)) setBorder(Yellow);
    else if (ack && (flash_state & 2)) setBorder(Yellow);
    else if(dispBorder)
    {
        drawLine(0, 0, 0, sy - 1, Black);
        drawLine(sx - 1, 0, sx - 1, sy - 1, Shadow);
        drawLine(0, 0, sx - 1, 0, Black);
        drawLine(0, sy - 1, sx - 1, sy - 1, Shadow);
    }
}
void Component::drawArc(float ang0, float ang1, float r, float cx, float cy)
{
    float xprev = r * cosf(ang0) + cx;
    float yprev = r * sinf(ang0) + cy;
    for(int i = 1; i < 101; i++)
    {
        float an = ang0 + (ang1 - ang0) * i / 100;
        float x = r * cosf(an) + cx;
        float y = r * sinf(an) + cy;
        drawLine(xprev, yprev, x, y);
        xprev = x;
        yprev = y;
    }
}
void Component::drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy)
{
    const int n = 51;
    float x[2 * n];
    float y[2 * n];
    for(int i = 0; i < n; i++)
    {
        float an = ang0 + (ang1 - ang0) * i / (n - 1);
        float c = cosf(an);
        float s = sinf(an);
        x[i] = rmin * c + cx;
        y[i] = rmin * s + cy;
        x[2 * n - 1 - i] = rmax * c + cx;
        y[2 * n - 1 - i] = rmax * s + cy;
    }
    drawPolygon(x, y, 2 * n);
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
            drawTexture(t->tex, t->x, t->y, t->width, t->height);
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
void Component::drawPolygon(float* x, float* y, int n)
{
    std::vector<short> scalex(n);
    std::vector<short> scaley(n);
    getXpoints(x, scalex.data(), n);
    getYpoints(y, scaley.data(), n);
    aapolygonRGBA(sdlren, scalex.data(), scaley.data(), n, renderColor.R, renderColor.G, renderColor.B, 255);
    filledPolygonRGBA(sdlren, scalex.data(), scaley.data(), n, renderColor.R, renderColor.G, renderColor.B, 255);
}
void Component::drawCircle(float radius, float cx, float cy)
{
    aacircleRGBA(sdlren, getX(cx), getY(cy), getScale(radius), renderColor.R, renderColor.G, renderColor.B, 255);
    filledCircleRGBA(sdlren, getX(cx), getY(cy), getScale(radius), renderColor.R, renderColor.G, renderColor.B, 255);
}
void Component::addRectangle(float x, float y, float w, float h, Color c, int align)
{
    add(new rectangle(x,y,w,h,c));
}
void Component::drawRectangle(float x, float y, float w, float h, Color c, int align)
{
    setColor(c);
    if(!(align & LEFT)) x = sx / 2 + x - w / 2;
    if(!(align & UP)) y = sy / 2 + y - h / 2;
    SDL_Rect r = {getX(x), getY(y), getScale(w), getScale(h)};
    int res = SDL_RenderFillRect(sdlren, &r);
    if(res < 0) printf("Failed to draw rectangle. SDL Error: %s\n", SDL_GetError());
}
void Component::drawRadius(float cx, float cy, float rmin, float rmax, float ang)
{
    float c = cosf(ang);
    float s = sinf(ang);
    drawLine(cx - rmin * c, cy - rmin * s, cx - rmax * c, cy - rmax * s);
}
void Component::drawTexture(std::shared_ptr<sdl_texture> tex, float cx, float cy, float sx, float sy)
{
    SDL_Rect rect = SDL_Rect({getX(cx - sx / 2), getY(cy - sy / 2), getScale(sx), getScale(sy)});
    SDL_RenderCopy(sdlren, tex->tex, nullptr, &rect);
}
void Component::addText(string text, float x, float y, float size, Color col, int align, int aspect)
{
    if(text=="") return;
    add(getText(text, x, y, size, col, align, aspect));
}
text_graphic* Component::getText(const string &text, float x, float y, float size, Color col, int align, int aspect)
{
    return getTextUnique(text, x, y, size, col, align, aspect).release();
}
std::unique_ptr<text_graphic> Component::getTextUnique(const string &text, float x, float y, float size, Color col, int align, int aspect)
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
    t->tex = getTextGraphic(text, size, col, aspect, align);
    float sx = t->tex == nullptr ? 0 : getAntiScale(t->tex->width);
    float sy = t->tex == nullptr ? 0 : getAntiScale(t->tex->height);
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
std::shared_ptr<sdl_texture> Component::getTextGraphic(string text, float size, Color col, int aspect, int align)
{
    if (text == "") return nullptr;
    TTF_Font *font = openFont(aspect&1 ? fontPathb : fontPath, size);
    TTF_SetFontWrappedAlign(font, align == CENTER ? TTF_WRAPPED_ALIGN_CENTER : (align == RIGHT ? TTF_WRAPPED_ALIGN_RIGHT : TTF_WRAPPED_ALIGN_LEFT));
    if (font == nullptr) return nullptr;
    //if (aspect & 2) TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
    SDL_Color color = {(Uint8)col.R, (Uint8)col.G, (Uint8)col.B};
    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, 0);
    auto *tex = new sdl_texture(SDL_CreateTextureFromSurface(sdlren, surf));
    tex->width = surf->w;
    tex->height = surf->h;
    SDL_FreeSurface(surf);
    return std::shared_ptr<sdl_texture>(tex);
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
        ig->width = ig->tex->width;
        ig->height = ig->tex->height;
        ig->x = this->sx/2;
        ig->y = this->sy/2;
    }
    return ig;
}
std::shared_ptr<sdl_texture> Component::getImageGraphic(string path)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    path = filesDir+"/"+path;
#endif
    SDL_Surface *surf = SDL_LoadBMP(path.c_str());
    if(surf == nullptr)
    {
        printf("Error loading BMP %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
        return nullptr;
    }
    SDL_Texture *t = SDL_CreateTextureFromSurface(sdlren, surf);
    if (t == nullptr) return nullptr;
    sdl_texture *tex = new sdl_texture(t);
    tex->width = surf->w;
    tex->height = surf->h;
    SDL_FreeSurface(surf);
    return std::shared_ptr<sdl_texture>(tex);
}
void Component::setBorder(Color c)
{
    setColor(c);
#if SIMRAIL
    SDL_Rect r;

    r = { getX(0), getY(0), getX(sx) - getX(0), getY(sy) - getY(0) };
    SDL_RenderDrawRect(sdlren, &r);

    r = { getX(1), getY(1), getX(sx) - getX(2), getY(sy) - getY(2) };
    SDL_RenderDrawRect(sdlren, &r);
#else
    SDL_Rect r = { getX(0), getY(0), getX(sx) - getX(0), getY(sy) - getY(0) };
    SDL_RenderDrawRect(sdlren, &r);
#endif
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

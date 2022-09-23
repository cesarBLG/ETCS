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
    if(bgColor != DarkBlue) drawRectangle(0, 0, sx, sy, bgColor);
    for(int i=0; i<graphics.size(); i++)
    {
        draw(graphics[i]);
    }
    if(display != nullptr) display();
    if(ack && (flash_state & 2)) setBorder(Yellow);
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
    int n = 51;
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
void Component::draw(graphic *graph, bool destroy)
{
    if(graph == nullptr) return;
    switch(graph->type)
    {
        case TEXTURE:{
            texture *t = (texture*)graph;
            if(t->tex == nullptr)
            {
                t->load();
            }
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
    if(destroy) delete graph;
}
void Component::drawPolygon(float *x, float *y, int n)
{
    short scalex[n];
    short scaley[n];
    getXpoints(x, scalex, n);
    getYpoints(y, scaley, n);
    aapolygonRGBA(sdlren, scalex, scaley, n, renderColor.R, renderColor.G, renderColor.B, 255);
    filledPolygonRGBA(sdlren, scalex, scaley, n, renderColor.R, renderColor.G, renderColor.B, 255);
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
void Component::drawTexture(SDL_Texture *tex, float cx, float cy, float sx, float sy)
{
    SDL_Rect rect = SDL_Rect({getX(cx - sx / 2), getY(cy - sy / 2), getScale(sx), getScale(sy)});
    SDL_RenderCopy(sdlren, tex, nullptr, &rect);
}
void Component::addText(string text, float x, float y, float size, Color col, int align, int aspect)
{
    add(getText(text,x,y,size,col,align,aspect));
}
text_graphic* Component::getText(string text, float x, float y, float size, Color col, int align, int aspect) 
{
    text_graphic *t = new text_graphic();
    t->text = text;
    t->offx = x;
    t->offy = y;
    t->size = size;
    t->color = col;
    t->alignment = align;
    t->aspect = aspect;
    t->load_function = [this, t]{getTextGraphic(t,t->text, t->offx, t->offy, t->size, t->color, t->alignment, t->aspect);};
    return t;
}
void Component::drawText(string text, float x, float y, float size, Color col, int align, int aspect)
{
    texture *t = new texture();
    getTextGraphic(t, text, x, y, size, col, align, aspect);
    draw(t,true);
}
void Component::getTextGraphic(texture *t, string text, float x, float y, float size, Color col, int align, int aspect)
{
    if(text=="") return;
    t->tex = nullptr;
    TTF_Font *font = openFont(aspect&1 ? fontPathb : fontPath, size);
    if (font == nullptr) return;
    if (aspect & 2) TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
    int v = text.find('\n');
    SDL_Color color = {(Uint8)col.R, (Uint8)col.G, (Uint8)col.B};
    getFontSize(font, text.substr(0,v).c_str(), &t->width, &t->height);
    float sx = t->width;
    float sy = t->height;
    if (align & UP) y = y + sy / 2;
    else if (align & DOWN) y = (this->sy - y) - sy / 2;
    else y = y + this->sy / 2;
    if (align & LEFT) x = x + sx / 2;
    else if (align & RIGHT) x = (this->sx - x) - sx / 2;
    else x = x + this->sx / 2;
    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, 0);
    if(surf==nullptr) printf("Error rendering text: %s\n", text.c_str());
    TTF_CloseFont(font);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlren, surf);
    t->tex = tex;
    t->x = x;
    t->y = y;
    t->width = getAntiScale(surf->w);
    t->height = getAntiScale(surf->h);
    SDL_FreeSurface(surf);
}
void Component::drawImage(string name, float cx, float cy, float sx, float sy)
{
    texture *t = new texture();
    getImageGraphic(t, name, cx, cy, sx, sy);
    draw(t,true);
}
void Component::addImage(string path, float cx, float cy, float sx, float sy)
{
    add(getImage(path,cx,cy,sx,sy));
}
image_graphic *Component::getImage(string path, float cx, float cy, float sx, float sy)
{
    image_graphic *ig = new image_graphic();
    ig->path = path;
    ig->cx = cx;
    ig->cy = cy;
    ig->sx = sx;
    ig->sy = sy;
    ig->load_function = [ig,this]{getImageGraphic(ig,ig->path,ig->cx, ig->cy, ig->sx, ig->sy);};
    return ig;
}
void Component::getImageGraphic(texture *t, string path, float cx, float cy, float sx, float sy)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    path = filesDir+"/"+path;
#endif
    SDL_Surface *surf = SDL_LoadBMP(path.c_str());
    if(surf == nullptr)
    {
        printf("Error loading BMP %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
        return;
    }
    if(sx > 0 && sy > 0)
    {
        t->width = sx;
        t->height = sy;
        t->x = cx;
        t->y = cy;
    }
    else
    {
        t->width = surf->w;
        t->height = surf->h;
        t->x = this->sx/2;
        t->y = this->sy/2;
    }
    t->tex = SDL_CreateTextureFromSurface(sdlren, surf);
    SDL_FreeSurface(surf);
}
void Component::setBorder(Color c)
{
    setColor(c);
    SDL_Rect r = {getX(0), getY(0), getX(sx) - getX(0), getY(sy) - getY(0)};
    SDL_RenderDrawRect(sdlren, &r);
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

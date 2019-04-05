#include "../component.h"
#include "../display.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <algorithm>
#include <cmath>
#include "../flash.h"
#include "../../sound/sound.h"
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

}
void Component::setPressedAction(function<void()> action)
{
    pressedAction = action;
}
void Component::setAck(function<void()> ackAction)
{
    pressedAction = ackAction;
    ack = ackAction!=nullptr;
}
void Component::setPressed(bool value)
{
    if(value && pressedAction != nullptr)
    {
        playClick();
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
    aalineRGBA(sdlren, getX(x1), getY(y1), getX(x2), getY(y2), renderColor.R, renderColor.G, renderColor.B, 255);
}
void Component::drawLine(float x1, float y1, float x2, float y2, Color c)
{
    if(x1 == x2) vlineRGBA(sdlren, getX(x1), getY(y1), getY(y1), c.R, c.G, c.B, 255);
    if(y1 == y2) hlineRGBA(sdlren, getX(x1), getX(x2), getY(y1), c.R, c.G, c.B, 255);
    aalineRGBA(sdlren, getX(x1), getY(y1), getX(x2), getY(y2), c.R, c.G, c.B, 255);
}
void Component::paint()
{
    if(bgColor != DarkBlue)
    {
        boxRGBA(sdlren, getX(0), getY(0), getX(sx), getY(sy), bgColor.R, bgColor.G, bgColor.B, 255);
    }
    if(display!=nullptr) display();
    if(ack && (flash_state & 2)) setBorder(Yellow);
    else if(dispBorder)
    {
        drawLine(0, 0, 0, sy-1, Black);
        drawLine(sx-1, 0, sx-1, sy-1, Shadow);
        drawLine(0, 0, sx-1, 0, Black);
        drawLine(0, sy-1, sx-1, sy-1, Shadow);
    }
}
void Component::drawArc(float ang0, float ang1, float r, float cx, float cy)
{
    float xprev = r*cosf(ang0)+cx;
    float yprev = r*sinf(ang0)+cy;
    for(int i=1; i<101; i++)
    {
        float an = ang0 + (ang1-ang0)*i/100;
        float x = r*cosf(an)+cx;
        float y = r*sinf(an)+cy;
        drawLine(xprev,yprev,x,y);
        xprev = x;
        yprev = y;
    }
}
void Component::drawSolidArc(float ang0, float ang1, float rmin, float rmax, float cx, float cy)
{
    int n = 51;
    float x[2*n];
    float y[2*n];
    for(int i=0; i<n; i++)
    {
        float an = ang0 + (ang1-ang0)*i/(n-1);
        float c = cosf(an);
        float s = sinf(an);
        x[i] = rmin*c + cx;
        y[i] = rmin*s + cy;
        x[2*n-1-i] = rmax*c + cx;
        y[2*n-1-i] = rmax*s + cy;
    }
    drawPolygon(x,y, 2*n);
}
void Component::rotateVertex(float *vx, float *vy, int pcount, float cx, float cy, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    for(int i = 0; i<pcount; i++)
    {
        float dx = vx[i];
        float dy = vy[i];
        vx[i] = cx-dy*c-dx*s;
        vy[i] = cy-dy*s+dx*c;
    }
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
void Component::drawBox(float sx, float sy, Color c)
{
    boxRGBA(sdlren, getX((this->sx-sx)/2), getY((this->sy-sy)/2), getX((this->sx+sx)/2), getY((this->sy+sy)/2), c.R, c.G, c.B, 255);
}
void Component::drawCircle(float radius, float cx, float cy)
{
    aacircleRGBA(sdlren, getX(cx), getY(cy), getScale(radius), renderColor.R, renderColor.G, renderColor.B, 255);
    filledCircleRGBA(sdlren, getX(cx), getY(cy), getScale(radius), renderColor.R, renderColor.G, renderColor.B, 255);
}
void Component::drawRadius(float cx, float cy, float rmin, float rmax, float ang)
{
    float c = cosf(ang);
    float s = sinf(ang);
    drawLine(cx-rmin*c, cy-rmin*s, cx-rmax*c, cy-rmax*s);
}
void Component::drawSurface(SDL_Surface *surf, float cx, float cy, float sx, float sy, bool destroy)
{
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdlren, surf);
    if(destroy) SDL_FreeSurface(surf);
    SDL_Rect rect = SDL_Rect({getX(cx - sx/2), getY(cy-sy/2), getScale(sx), getScale(sy)});
    SDL_RenderCopy(sdlren, tex, nullptr, &rect);
    SDL_DestroyTexture(tex);
}
void Component::drawImage(const char *name, float cx, float cy, float sx, float sy)
{
    drawSurface(SDL_LoadBMP(name),cx,cy,sx,sy);
}
void Component::drawText(const char *text, float x, float y, float sx, float sy, float size, Color col, int align, int aspect)
{
    /*if(surfloc != text)
    {
        if(bgSurf != nullptr) SDL_FreeSurface(bgSurf);
        surfloc = text;
    }*/
    TTF_Font *font = openFont(aspect ? fontPathb : fontPath, size == 0 ? 54 : size);
    SDL_Color color = {col.R, col.G, col.B};
    float width;
    float height;
    getFontSize(font, text, &width, &height);
    sx = sx == 0 ? width : min(sx, width);
    sy = sy == 0 ? height : min(sy, height);
    if(align & UP) y = y + sy/2;
    else if(align & DOWN) y = (this->sy-y) - sy/2;
    else y = y + this->sy/2;
    if(align & LEFT) x = x + sx/2;
    else if(align & RIGHT) x = (this->sx-x) - sx/2;
    else x = x + this->sx/2;
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    TTF_CloseFont(font);
    drawSurface(surf, x, y, sx, sy, true);
}
void Component::setBackgroundImage(const char *name)
{
    /*if(surfloc != name)
    {
        if(bgSurf != nullptr) SDL_FreeSurface(bgSurf);
        bgSurf = SDL_LoadBMP(name);
        surfloc = name;
    }*/
    bgSurf = SDL_LoadBMP(name);
    drawSurface(bgSurf,sx/2,sy/2,bgSurf->w,bgSurf->h,true);
}
void Component::setText(const char* text, float size, Color c)
{
    string t = text;
    int v = t.find('\n');
    if(v!=string::npos)
    {
        TTF_Font *font = openFont(fontPath, size);
        float width;
        float height;
        string t1 = t.substr(0,v);
        string t2 = t.substr(v+1);
        getFontSize(font,text, &width, &height);
        TTF_CloseFont(font);
        drawText(t1.c_str(), 0, -height/2, sx, sy, size, c, CENTER);
        drawText(t2.c_str(), 0, height/2, sx, sy, size, c, CENTER);
    }
    else drawText(text, 0, 0, sx, sy, size, c, CENTER);
}
void Component::setBorder(Color c)
{
    rectangleRGBA(sdlren, getX(0), getY(0), getX(sx), getY(sy), c.R, c.G, c.B, 255);
}
void Component::setBackgroundColor(Color c)
{
    bgColor = c;
}
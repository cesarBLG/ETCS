#include "button.h"
#include <SDL2/SDL2_gfxPrimitives.h>
void Button::paint()
{
    if(enabled)
    {
        vlineRGBA(sdlren, getX(2), getY(2), getY(sy-3), Shadow.R, Shadow.G, Shadow.B, 255);
        vlineRGBA(sdlren, getX(sx-3), getY(2), getY(sy-3), Black.R, Black.G, Black.B, 255);
        hlineRGBA(sdlren, getX(2), getX(sx-3), getY(2), Shadow.R, Shadow.G, Shadow.B, 255);
        hlineRGBA(sdlren, getX(2), getX(sx-3), getY(sy-3), Black.R, Black.G, Black.B, 255);
    }
    Component::paint();
}
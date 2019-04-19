#include "button.h"
void Button::paint()
{
    if(enabled&&showBorder)
    {
        drawLine(2, 2, 2, sy-3, Shadow);
        drawLine(sx-3, 2, sx-3, sy-3, Black);
        drawLine(2, 2, sx-3, 2, Shadow);
        drawLine(2, sy-3, sx-3, sy-3, Black);
    }
    Component::paint();
}
#include "window.h"
void window::event(int evNo, float x, float y)
{
    vector<LayoutElement>& el = getLayoutElements();
    for(int i=0; i<el.size(); i++)
    {
        Component *comp = el[i].comp;
        if((comp->x-comp->touch_left)<x && (comp->x + comp->sx + comp->touch_right)>x
        && (comp->y-comp->touch_up)<y && (comp->y + comp->sy+comp->touch_down)>y)
        {
            if(comp->isSensitive()) comp->setPressed(true);
        }
    }
}
void window::display()
{
    l->update();
}
window::~window()
{
    delete l;
}
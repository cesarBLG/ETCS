#include "window.h"
void window::event(int evNo, float x, float y)
{
    vector<LayoutElement>& el = getLayoutElements();
    for(int i=0; i<el.size(); i++)
    {
        Component *comp = el[i].comp;
        if(comp->x<x && (comp->x + comp->sx)>x
        && comp->y<y && (comp->y + comp->sy)>y)
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
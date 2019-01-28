#ifndef _WINDOW_H
#define _WINDOW_H
#include "layout.h"
class window
{
    Layout *l;
    public:
    window()
    {
        l = new Layout();
    }
    ~window()
    {
        delete l;
    }
    vector<LayoutElement>& getLayoutElements()
    {
        return l->getElements();
    }
    void addToLayout(Component *comp, ComponentAlignment *alignment) 
    {
        l->add(comp, alignment);
    }
    void display()
    {
        l->update();
    }
    void event(int evNo, float x, float y)
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
};
#endif

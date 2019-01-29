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
    ~window();
    vector<LayoutElement>& getLayoutElements()
    {
        return l->getElements();
    }
    void addToLayout(Component *comp, ComponentAlignment *alignment) 
    {
        l->add(comp, alignment);
    }
    void display();
    void event(int evNo, float x, float y);
};
#endif

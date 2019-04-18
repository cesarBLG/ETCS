#ifndef _WINDOW_H
#define _WINDOW_H
#include "../graphics/layout.h"
#include "../control/control.h"
class window
{
    Layout *l;
    function<void()> constructfun;
    public:
    bool active = true;
    window(function<void()> constructfun = nullptr) : constructfun(constructfun)
    {
        l = new Layout();
    }
    virtual ~window();
    vector<LayoutElement>& getLayoutElements()
    {
        return l->getElements();
    }
    void addToLayout(Component *comp, ComponentAlignment *alignment) 
    {
        l->add(comp, alignment);
    }
    void construct()
    {
        if(constructfun != nullptr) constructfun();
    }
    void display();
    void event(int evNo, float x, float y);
};
#endif

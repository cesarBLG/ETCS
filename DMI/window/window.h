/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _WINDOW_H
#define _WINDOW_H
#include "../language/language.h"
#include "../graphics/layout.h"
#include "../control/control.h"
class window
{
    protected:
    Layout *l;
    std::function<void(window*)> constructfun;
    public:
    bool active = true;
    Component *pressedButton = nullptr;
    window(std::function<void(window*)> constructfun = nullptr) : constructfun(constructfun)
    {
        l = new Layout();
    }
    virtual ~window();
    std::vector<LayoutElement>& getLayoutElements()
    {
        return l->getElements();
    }
    void addToLayout(Component *comp, ComponentAlignment *alignment) 
    {
        l->add(comp, alignment);
    }
    void remove(Component *comp)
    {
        l->remove(comp);
    }
    void bringFront(Component *comp)
    {
        l->order.push_back(comp);
    }
    void clearLayout()
    {
        l->removeAll();
    }
    void construct()
    {
        clearLayout();
        if(constructfun != nullptr) constructfun(this);
    }
    void display();
    void event(int evNo, float x, float y);
    void updateLayout();
};
#endif

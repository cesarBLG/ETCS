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
        if(constructfun != nullptr) constructfun(this);
    }
    void display();
    void event(int evNo, float x, float y);
};
#endif

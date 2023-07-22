/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "layout.h"
#include <vector>
using namespace std;
void Layout::add(Component *comp, ComponentAlignment *alignment)
{
    if(comp == nullptr || alignment == nullptr) return;
    elements.push_back({comp, alignment});
    if(alignment->layer==0) comp->dispBorder = false;
    updateLocations();
}
void Layout::remove(Component *comp)
{
    if(comp == nullptr) return;
    for(auto it = elements.begin(); it != elements.end(); )
    {
        if (it->comp == comp)
        {
            delete it->alignment;
            it = elements.erase(it);
            continue;
        }
        ++it;
    }
}
void Layout::removeAll()
{
    for(int i=0; i<elements.size(); i++)
    {
        delete elements[i].alignment;
    }
    elements.clear();
}
void Layout::update()
{   
    if(!order.empty())
    {
        for(int i=0; i<elements.size(); i++)
        {
            order[i]->paint();
        }
        return;
    }
    for(int i=0; i<elements.size(); i++)
    {
        elements[i].comp->paint();
    }
}
void Layout::updateLocations()
{
    for(int i=0; i<elements.size(); i++)
    {
        Component *c = elements[i].comp;
        ComponentAlignment *align = elements[i].alignment;
        if(align->alignType == AlignType::RELATIVE_ALIGN)
        {
            RelativeAlignment *offset = (RelativeAlignment *)align;
            if(align->relative!=nullptr)
            {
                c->setLocation(offset->x + align->relative->x - c->sx/2, offset->y + align->relative->y - c->sy/2);
            }
            else c->setLocation(offset->x, offset->y);
        }
        if(align->alignType == AlignType::CONSECUTIVE_ALIGN)
        {
            ConsecutiveAlignment *offset = (ConsecutiveAlignment *)align;
            int al = offset->align;
            float x = align->relative->x;
            float y = align->relative->y;
            if(al & RIGHT) x+=align->relative->sx;
            else if(!(al & LEFT) && (al & DOWN)) x += (align->relative->sx - c->sx)/2;
            else if((al & LEFT) && !(al & DOWN)) x-=c->sx;
            if(al & DOWN) y+=align->relative->sy;
            else if(!(al & UP) && (al & RIGHT)) y += (align->relative->sy - c->sy)/2;
            else if((al & UP) && !(al & RIGHT)) y-=c->sy;
            c->setLocation(x,y);
        }
    }
}
vector<LayoutElement>& Layout::getElements()
{
    return elements;
}
Layout::~Layout()
{
    removeAll();
}
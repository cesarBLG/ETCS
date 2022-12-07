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
#include "layout.h"
#include <vector>
#include <chrono>
#include <cstdio>
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
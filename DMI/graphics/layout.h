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
#ifndef _LAYOUT_H
#define _LAYOUT_H
#include "component.h"
#include <vector>
using namespace std;
enum AlignType
{
    CONSECUTIVE,
    RELATIVE
};
struct ComponentAlignment
{
    AlignType alignType;
    Component *relative;
    int layer;
    ComponentAlignment(AlignType at, Component *c, int layer = -1) : alignType(at), relative(c), layer(layer)
    {
    }
};
struct ConsecutiveAlignment : public ComponentAlignment
{
    int align;
    ConsecutiveAlignment(Component *rel, int align, int layer = -1) : ComponentAlignment(CONSECUTIVE, rel, layer)
    {
        this->align = align;
    }
};
struct RelativeAlignment : public ComponentAlignment
{
    float x;
    float y;
    RelativeAlignment(Component *rel, float x, float y, int layer = -1) : ComponentAlignment(RELATIVE, rel, layer)
    {
        this->x = x;
        this->y = y;
    }
};
struct LayoutElement
{
    Component *comp;
    ComponentAlignment *alignment;
};
class Layout
{
    vector<LayoutElement> elements;
    public:
    vector<Component*> order;
    void add(Component *comp, ComponentAlignment *alignment);
    void removeAll();
    void update();
    void updateLocations();
    vector<LayoutElement>& getElements();
};
#endif
/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _LAYOUT_H
#define _LAYOUT_H
#include "component.h"
#include <vector>
enum struct AlignType
{
    CONSECUTIVE_ALIGN,
    RELATIVE_ALIGN
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
    ConsecutiveAlignment(Component *rel, int align, int layer = -1) : ComponentAlignment(AlignType::CONSECUTIVE_ALIGN, rel, layer)
    {
        this->align = align;
    }
};
struct RelativeAlignment : public ComponentAlignment
{
    float x;
    float y;
    RelativeAlignment(Component *rel, float x, float y, int layer = -1) : ComponentAlignment(AlignType::RELATIVE_ALIGN, rel, layer)
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
    std::vector<LayoutElement> elements;
    public:
    ~Layout();
    std::vector<Component*> order;
    void add(Component *comp, ComponentAlignment *alignment);
    void remove(Component *comp);
    void removeAll();
    void update();
    void updateLocations();
    std::vector<LayoutElement>& getElements();
};
#endif

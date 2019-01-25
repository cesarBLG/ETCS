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
    ComponentAlignment(AlignType at, Component *c)
    {
        alignType = at;
        relative = c;
    }
};
struct ConsecutiveAlignment : public ComponentAlignment
{
    int align;
    ConsecutiveAlignment(Component *rel, int align) : ComponentAlignment(CONSECUTIVE, rel)
    {
        this->align = align;
    }
};
struct RelativeAlignment : public ComponentAlignment
{
    float x;
    float y;
    RelativeAlignment(Component *rel, float x, float y) : ComponentAlignment(RELATIVE, rel)
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
    void add(Component *comp, ComponentAlignment *alignment);
    void update();
    void updateLocations();
    vector<LayoutElement>& getElements();
};
#endif
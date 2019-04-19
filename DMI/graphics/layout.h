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
    void update();
    void updateLocations();
    vector<LayoutElement>& getElements();
};
#endif
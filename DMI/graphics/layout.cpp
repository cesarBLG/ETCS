#include "layout.h"
#include <vector>
#include <chrono>
#include <cstdio>
using namespace std;
static vector<LayoutElement> elements;
void Layout::add(Component *comp, ComponentAlignment *alignment)
{
    elements.push_back({comp, alignment});
    updateLocations();
}
void Layout::update()
{
    auto start = chrono::system_clock::now();
    for(int i=0; i<elements.size(); i++)
    {
        elements[i].comp->paint();
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end-start;
    printf("%f\n", diff.count());
}
void Layout::updateLocations()
{
    for(int i=0; i<elements.size(); i++)
    {
        Component *c = elements[i].comp;
        ComponentAlignment *align = elements[i].alignment;
        if(align->alignType == RELATIVE)
        {
            RelativeAlignment *offset = (RelativeAlignment *)align;
            if(align->relative!=nullptr)
            {
                c->setLocation(offset->x + align->relative->x - c->sx/2, offset->y + align->relative->y - c->sy/2);
                //c->layer = align->relative->layer - 1;
            }
            else c->setLocation(offset->x, offset->y);
        }
        if(align->alignType == CONSECUTIVE)
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
            //if(align->relative->layer != 0) c->layer = align->relative->layer;
            c->setLocation(x,y);
        }
    }
}
vector<LayoutElement>& Layout::getElements()
{
    return elements;
}
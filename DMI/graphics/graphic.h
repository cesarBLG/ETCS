#ifndef _GRAPHIC_H
#define _GRAPHIC_H
#include "color.h"
using namespace std;
enum graphic_type
{
    TEXTURE,
    RECTANGLE,
    LINE,
    CIRCLE,
    SOLID_ARC
};
class graphic
{
    public:
    graphic_type type;
    graphic(graphic_type type) : type(type){}
    virtual ~graphic()
    {
        
    }
};
#endif
#ifndef _TEXT_GRAPHIC_H
#define _TEXT_GRAPHIC_H
#include "texture.h"
#include <string>
using namespace std;
class text_graphic : public texture
{
    public:
    string text;
    float size;
    float offx;
    float offy;
    int alignment;
    int aspect;
    Color color;
};
#endif
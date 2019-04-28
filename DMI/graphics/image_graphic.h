#ifndef _IMAGE_GRAPHIC_H
#define _IMAGE_GRAPHIC_H
#include "texture.h"
#include <string>
using namespace std;
class image_graphic : public texture
{
    public:
    string path;
    float sx;
    float sy;
    float cx;
    float cy;
};
#endif
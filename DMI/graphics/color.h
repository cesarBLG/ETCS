#ifndef _COLOR_H
#define _COLOR_H
struct Color
{
    int R;
    int G;
    int B;
    bool operator==(Color b)
    {
        return R == b.R && G==b.G && B==b.B;
    }
    bool operator!=(Color b)
    {
        return !(*this==b);
    }
};
const Color White = {255,255,255};
const Color Black = {0,0,0};
const Color Grey = {195,195,195};
const Color MediumGrey = {150,150,150};
const Color DarkGrey = {85,85,85};
const Color Yellow = {223,223,0};
const Color Shadow = {8, 24, 57};
const Color Red = {191,0,2};
const Color Orange = {234,145,0};
const Color DarkBlue = {3, 17, 34};
const Color PASPdark = {33, 49, 74};
const Color PASPlight = {41, 74, 107};
#endif
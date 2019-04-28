#include "../graphics/component.h"
#include "../monitor.h"
#include "level.h"
#include <string>
using namespace std;
extern Component distanceBar;
Component a4(54, 25, nullptr);
Component levelRegion(54, 25, displayLevel);
static Level prevlevel;
void displayLevel()
{
    if(prevlevel==level) return;
    prevlevel = level;
    levelRegion.clear();
    string path = "symbols/Level/LE_";
    int num = 0;
    switch(level)
    {
        case N0:
            num=1;
            break;
        case NTC:
            num=2;
            break;
        case N1:
            num=3;
            break;
        case N2:
            num=4;
            break;
        case N3:
            num=5;
            break;
    }
    if(num<10) path+="0";
    path+=to_string(num);
    //If NTC is LZB/PZB, path+="a";
    path+=".bmp";
    levelRegion.addImage(path);
}
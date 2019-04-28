#include "mode.h"
#include "../monitor.h"
#include "../graphics/component.h"
#include <string>
using namespace std;
extern Component csg;
Component modeRegion(36,36, displayMode);
static Mode prevmode;
void displayMode()
{
    if(mode==prevmode) return;
    prevmode = mode;
    modeRegion.clear();
    string path = "symbols/Mode/MO_";
    int num;
    switch(mode)
    {
        case SH:
            num = 1;
            break;
        case TR:
            num = 4;
            break;
        case PT:
            num = 6;
            break;
        case OS:
            num = 7;
            break;
        case SR:
            num = 9;
            break;
        case FS:
            num = 11;
            break;
        case NL:
            num = 12;
            break;
        case SB:
            num = 13;
            break;
        case RV:
            num = 14;
            break;
        case UN:
            num = 16;
            break;
        case SF:
            num = 18;
            break;
        case NS:
            num = 19;
            break;
        case LS:
            num = 21;
            break;
    }
    if(num<10) path+="0";
    path+=to_string(num);
    path+=".bmp";
    modeRegion.addImage(path);
}
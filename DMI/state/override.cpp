#include "override.h"
#include "../monitor.h"
void displayOverride();
Component c7(37, 50, displayOverride);
void displayOverride()
{
    if(ovEOA) c7.setBackgroundImage("symbols/Mode/MO_03.bmp");
}
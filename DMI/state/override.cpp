#include "override.h"
#include "../monitor.h"
void displayOverride();
Component c7(37, 50, displayOverride);
static bool prevOverride;
void displayOverride()
{
    if(prevOverride == ovEOA) return;
    prevOverride = ovEOA;
    c7.clear();
    if(ovEOA) c7.addImage("symbols/Mode/MO_03.bmp");
}
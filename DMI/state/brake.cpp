#include "../graphics/component.h"
#include "../monitor.h"
void displayBrake();
Component c9(54, 25, displayBrake);
static bool prevEB;
void displayBrake()
{
    if(prevEB == EB) return;
    prevEB = EB;
    c9.clear();
    if(EB) c9.addImage("symbols/Status/ST_01.bmp");
}
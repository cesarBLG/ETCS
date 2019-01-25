#include "../graphics/component.h"
#include "../monitor.h"
void displayBrake();
Component c9(54, 25, displayBrake);
void displayBrake()
{
    if(EB) c9.setBackgroundImage("symbols/Status/ST_01.bmp");
}
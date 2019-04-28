#include "gps_pos.h"
#include "../time.h"
Component gps_pos(120,50,display_gps);
static bool f = false;
void display_gps()
{
    if(f) return;
    f = true;
    gps_pos.addImage("symbols/Driver Request/DR_03.bmp");
}

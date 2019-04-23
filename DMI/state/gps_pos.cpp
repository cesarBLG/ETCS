#include "gps_pos.h"
#include "../time.h"
Component gps_pos(120,50,display_gps);
void display_gps()
{
    gps_pos.setBackgroundImage("symbols/Driver Request/DR_03.bmp");
}

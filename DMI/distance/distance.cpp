#include "../graphics/component.h"
#include "../monitor.h"
#include "distance.h"
#include <string>
using namespace std;
const int posy[] = {-1,6,13,22,32,45,59,79,105,152,185};
const int posx[] = {12,16,16,16,16,12,16,16,16,16,12};
extern Component a1;
Component a2(54,30, displayDistanceText);
Component distanceBar(54,191, displayDistance);
Component a23(54, 221, nullptr);
extern bool showSpeeds;
static float prev_dist = 0;
void displayDistanceText()
{
    float dist = Dtarg;
    if(dist!=prev_dist)
    {
        a2.clear();
        if(monitoring == CSM && Vtarget>=Vperm) return;
        if(!showSpeeds && (mode == OS || mode == SR)) return;
        const char *str = to_string(((((int)(dist))/10))*10).c_str();
        a2.addText(str, 10, 0, 10, Grey, RIGHT);
        prev_dist = dist;
    }
}
void displayDistance()
{
    float dist = Dtarg;
    if(monitoring == CSM && Vtarget>=Vperm) return;
    if(!showSpeeds && (mode == OS || mode == SR)) return;
    for(int i=0; i<11; i++)
    {
        int dist = 1000-i*100;
        distanceBar.drawRectangle(posx[i], posy[i], 25-posx[i], 1, Grey);
    }
    if(dist>1000) dist = 1000;
    float h = 0;
    if(dist<100) h = dist/100*(185-152);
    else
    {
        h = 185-152;
        h += (log10(dist)-2)*(152+1);
    }
    distanceBar.drawRectangle(29, 186-h, 10, h, Grey);
    /*for(float y = 186; y>=(186-h); y-=0.5)
    {
        distanceBar.drawLine(29, y, 39, y);
    }*/
}
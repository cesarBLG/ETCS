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
void displayDistanceText()
{
    float dist = Dtarg;
    if(monitoring == CSM && Vtarget>=Vperm) return;
    if(!showSpeeds && (mode == OS || mode == SR)) return;
    const char *str = to_string(((((int)(dist))/10))*10).c_str();
    a2.drawText(str, 10, 0, 0, 0, 10, Grey, RIGHT);
}
void displayDistance()
{
    float dist = Dtarg;
    if(monitoring == CSM && Vtarget>=Vperm) return;
    if(!showSpeeds && (mode == OS || mode == SR)) return;
    setColor(Grey);
    for(int i=0; i<11; i++)
    {
        int dist = 1000-i*100;
        distanceBar.drawLine(posx[i], posy[i], 25, posy[i]);
        distanceBar.drawLine(posx[i], posy[i]+0.5, 25, posy[i]+0.5);
        distanceBar.drawLine(posx[i], posy[i]+1, 25, posy[i]+1);
    }
    if(dist>1000) dist = 1000;
    float h = 0;
    if(dist<100) h = dist/100*(185-152);
    else
    {
        h = 185-152;
        h += (log10(dist)-2)*(152+1);
    }
    for(float y = 186; y>=(186-h); y-=0.5)
    {
        distanceBar.drawLine(29, y, 39, y);
    }
}
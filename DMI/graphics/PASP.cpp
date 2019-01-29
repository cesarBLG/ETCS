#include "PASP.h"
#include "../graphics/component.h"
#include "../monitor.h"
#include <string>

const int posy[] = {283,250,206,182,164,150,107,64,21};
const int objectPos[] = {55,80,105};
Component distancePASP(246,300, displayPASP);
extern bool showSpeeds;
void displayPASP()
{
    setColor(DarkGrey);
    for(int i=0; i<9; i++)
    {
        int dist = 1000-i*100;
        distancePASP.drawLine(40, posy[i], 240, posy[i]);
        distancePASP.drawLine(40, posy[i]+0.5, 240, posy[i]+0.5);
        distancePASP.drawLine(40, posy[i]+1, 240, posy[i]+1);
    }
    distancePASP.drawImage("symbols/Track Conditions/TC_01.bmp",objectPos[0],20,20,20);
    distancePASP.drawImage("symbols/Track Conditions/TC_04.bmp",objectPos[1],40,20,20);
    distancePASP.drawImage("symbols/Track Conditions/TC_06.bmp",objectPos[2],60,20,20);
}

float getPASPy(float dist)
{
    if(dist<25) return 283-((283-250)/25)*dist;
    else return 250-(250-21)/log10(1000/25)*log10(dist/25);
}
#include "PASP.h"
#include "../graphics/component.h"
#include "../monitor.h"
#include <string>
#include "../graphics/icon_button.h"

const int posy[] = {283,250,206,182,164,150,107,64,21};
const int divs[] = {0, 25, 50, 75, 100, 125, 250, 500, 1000};
int pasp_scale = 8;
const int objectPos[] = {55,80,105};
Component distancePASP(246,300, displayPASP);
void displayScaleUp();
void displayScaleDown();
void zoominp(){pasp_scale/=2;}
void zoomoutp(){pasp_scale*=2;}
IconButton zoomin("symbols/Navigation/NA_03.bmp",40,15,zoominp);
IconButton zoomout("symbols/Navigation/NA_04.bmp",40,15,zoomoutp);
void displayPASP()
{
    setColor(DarkGrey);
    for(int i=0; i<9; i++)
    {
        if(i==0||i>4)
        {
            distancePASP.drawText(to_string(divs[i]*pasp_scale).c_str(), 208, posy[i]-150, 0,0, 10, White, RIGHT);
        }
        distancePASP.drawLine(40, posy[i], 240, posy[i]);
        if(i==0||i==5||i==8)
        {
            distancePASP.drawLine(40, posy[i]+0.5, 240, posy[i]+0.5);
        }
    }
    distancePASP.drawImage("symbols/Track Conditions/TC_01.bmp",objectPos[0],getPASPy(120),20,20);
    distancePASP.drawImage("symbols/Track Conditions/TC_04.bmp",objectPos[1],getPASPy(26000),20,20);
    distancePASP.drawImage("symbols/Track Conditions/TC_06.bmp",objectPos[2],getPASPy(3000),20,20);
}

float getPASPy(float dist)
{
    int first_line = divs[1]*pasp_scale;
    if(dist<first_line) return posy[0]-((posy[0]-posy[1])/first_line)*dist;
    else return posy[1]-(posy[1]-posy[8])/log10(divs[8]*pasp_scale/first_line)*log10(dist/first_line);
}
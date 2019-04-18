#include "planning.h"
#include "../graphics/component.h"
#include "../monitor.h"
#include <string>
#include "../graphics/icon_button.h"
#include "../window/window.h"
#include "../graphics/display.h"

void planningConstruct();
window planning_area(planningConstruct);
const int posy[] = {283,250,206,182,164,150,107,64,21};
const int divs[] = {0, 25, 50, 75, 100, 125, 250, 500, 1000};
int planning_scale = 8;
const int object_pos[] = {55,80,105};
Component planning_distance(246,300, displayPlanning);
void displayScaleUp();
void displayScaleDown();
void zoominp()
{
    if(planning_scale>1) planning_scale/=2;
}
void zoomoutp()
{
    if(planning_scale<=16) planning_scale*=2;
}
IconButton zoomin("symbols/Navigation/NA_03.bmp",40,15,zoominp);
IconButton zoomout("symbols/Navigation/NA_04.bmp",40,15,zoomoutp);
vector<planning_element> planning_elements;
void displayPlanning()
{
    setColor(DarkGrey);
    for(int i=0; i<9; i++)
    {
        if(i==0||i>4)
        {
            planning_distance.drawText(to_string(divs[i]*planning_scale).c_str(), 208, posy[i]-150, 0,0, 10, White, RIGHT);
        }
        planning_distance.drawLine(40, posy[i], 240, posy[i]);
        if(i==0||i==5||i==8)
        {
            planning_distance.drawLine(40, posy[i]+0.5, 240, posy[i]+0.5);
        }
    }
    for(int i = 0; i < planning_elements.size(); i++)
    {
        planning_element p = planning_elements[i];
        string name = string("symbols/Track Conditions/TC_") + (p.condition < 10 ? "0" : "") + to_string(p.condition)+".bmp";
        planning_distance.drawImage(name.c_str(),object_pos[i%3],getPlanningHeight(p.distance),20,20);
    }
}
void planningConstruct()
{
    planning_area.addToLayout(&planning_distance, new RelativeAlignment(nullptr, 334,15));
    planning_area.addToLayout(&zoomin, new RelativeAlignment(&planning_distance, 20,8,0));
    planning_area.addToLayout(&zoomout, new RelativeAlignment(&planning_distance, 20,292,0));
}
float getPlanningHeight(float dist)
{
    int first_line = divs[1]*planning_scale;
    if(dist<first_line) return posy[0]-((posy[0]-posy[1])/first_line)*dist;
    else return posy[1]-(posy[1]-posy[8])/log10(divs[8]*planning_scale/first_line)*log10(dist/first_line);
}
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
int planning_scale = 1;
const int object_pos[] = {55,80,105};
Component planning_distance(246,300, displayPlanning);
Component planning_gradient(18,270, displayGradient);
Component PASP(99,270, displayPASP);
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
        planning_distance.drawLine(40, posy[i], 240-1, posy[i]);
        if(i==0||i==5||i==8)
        {
            planning_distance.drawLine(40, posy[i]+0.5, 240-1, posy[i]+0.5);
        }
    }
    for(int i = 0; i < planning_elements.size(); i++)
    {
        planning_element p = planning_elements[i];
        string name = string("symbols/Track Conditions/TC_") + (p.condition < 10 ? "0" : "") + to_string(p.condition)+".bmp";
        planning_distance.drawImage(name.c_str(),object_pos[i%3],getPlanningHeight(p.distance),20,20);
    }
}
vector<gradient_element> gradient_elements;
void displayGradient()
{
    for(int i=0; i<gradient_elements.size(); i++)
    {
        gradient_element &e = gradient_elements[i];
        if(e.distance>divs[8]*planning_scale) continue;
        float max = e.distance+e.length;
        float minp = getPlanningHeight(max)-15;
        float maxp = getPlanningHeight(e.distance)-15;
        if(max>divs[8]*planning_scale) minp = 0;
        float size = maxp-minp;
        planning_gradient.drawRectangle(0, minp, 18, size, e.val<0 ? Grey : DarkGrey);
        planning_gradient.drawLine(0, minp, 17, minp, e.val<0 ? White : Grey);
        planning_gradient.drawLine(0, minp, 0, maxp-1, e.val<0 ? White : Grey);
        planning_gradient.drawLine(0, maxp-1, 17, maxp-1, Black);
        if(size>44)
        {
            planning_gradient.drawText(to_string(abs(e.val)).c_str(), 0, (minp+maxp-1)/2-planning_gradient.sy/2, 0, 0, 10, e.val<0 ? Black : White);
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, minp-planning_gradient.sy/2+7, 0,0, 10, e.val<0 ? Black : White);
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, maxp-planning_gradient.sy/2-8, 0,0, 10, e.val<0 ? Black : White);
        }
        else if(size>14)
        {
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, (minp+maxp-1)/2-planning_gradient.sy/2, 0,0, 10, e.val<0 ? Black : White);
        }
    }
}
vector<speed_element> speed_elements;
bool check_spdov(int i, int j)
{
    speed_element &cur = speed_elements[i];
    if(i+1>=speed_elements.size() || j>=speed_elements.size()) return false;
    speed_element &chk = speed_elements[j];
    float a = getPlanningHeight(cur.distance)-15;
    float b = getPlanningHeight(chk.distance)-15;
    if(a-b>20) return false;
    return cur.speed>chk.speed || check_spdov(i, j+1);
}
void displayPASP()
{
    PASP.drawRectangle(14, 0, 99, 270, PASPdark);

    speed_element prev_pasp = speed_elements[0];
    bool oth1 = false;
    bool oth2 = false;
    float red = 1;
    float spd = speed_elements[0].speed;
    bool end = false;
    for(int i=1; i<speed_elements.size(); i++)
    {
        speed_element cur = speed_elements[i];
        speed_element prev = speed_elements[i-1];
        if(cur.distance>divs[8]*planning_scale)
        {
            PASP.drawRectangle(14, 0, 93*red, getPlanningHeight(prev_pasp.distance)-15, PASPlight);
            end = true;
            break;
        }
        if(prev_pasp.speed>cur.speed && (!oth2||cur.speed==0))
        {
            printf("%d",i);
            oth1 = true;
            PASP.drawRectangle(14, getPlanningHeight(cur.distance)-15, 93*red, getPlanningHeight(prev_pasp.distance)-getPlanningHeight(cur.distance), PASPlight);
            float v = cur.speed/spd;
            if(v>0.74) red = 3.0/4;
            else if(v>0.49) red = 1.0/2;
            else red = 1.0/4;
            if(cur.speed == 0)
            {
                end = true;
                break;
            }
            prev_pasp = cur;
        }
        //if(oth1 && prev.speed<cur.speed) oth2 = true;
    }
    if(!end)
    {
        PASP.drawRectangle(14, 0, 93*red, getPlanningHeight(prev_pasp.distance)-15, PASPlight);
    }

    int ld = 0;
    for(int i=1; i<speed_elements.size(); i++)
    {
        if(check_spdov(i, i+1)) continue;
        speed_element cur = speed_elements[i];
        speed_element prev = speed_elements[ld];
        ld = i;
        if(cur.distance>divs[8]*planning_scale) break;
        float a = getPlanningHeight(cur.distance)-15;
        PASP.drawLine(6, a-1, 22, a-1, Grey);
        PASP.drawLine(6, a, 22, a, Grey);
        float x[] = {15, 18, 21};
        if(prev.speed>cur.speed)
        {
            float y[] = {a+2, a+10, a+2};
            PASP.drawPolygon(x, y, 3);
            PASP.drawText(to_string(cur.speed).c_str(), 25, a-2, 0, 0, 10, Grey, UP | LEFT);
        }
        else
        {
            float y[] = {a-3, a-11, a-3};
            PASP.drawPolygon(x, y, 3);
            PASP.drawText(to_string(cur.speed).c_str(), 25, 270-a-2, 0, 0, 10, Grey, DOWN | LEFT);
        }
    }
}
void planningConstruct()
{
    planning_area.addToLayout(&planning_distance, new RelativeAlignment(nullptr, 334,15));
    planning_area.addToLayout(&zoomin, new RelativeAlignment(&planning_distance, 20,8,0));
    planning_area.addToLayout(&zoomout, new RelativeAlignment(&planning_distance, 20,292,0));
    planning_area.addToLayout(&planning_gradient, new RelativeAlignment(&planning_distance, 115+9, 15+135, 0));
    planning_area.addToLayout(&PASP, new ConsecutiveAlignment(&planning_gradient, RIGHT, 0));
    zoomin.showBorder = false;
    zoomout.showBorder = false;
    zoomin.touch_down = 15;
    zoomout.touch_up = 15;
    gradient_elements.push_back({10,0,600});
    gradient_elements.push_back({-5,600,3000});
    speed_elements.push_back({120, 0});
    speed_elements.push_back({90, 50});
    speed_elements.push_back({60, 200});
    speed_elements.push_back({20, 300});
    speed_elements.push_back({120, 500});
    speed_elements.push_back({0, 800});
}
float getPlanningHeight(float dist)
{
    float first_line = divs[1]*planning_scale;
    if(dist<first_line) return posy[0]-((posy[0]-posy[1])/first_line)*dist;
    else return posy[1]-(posy[1]-posy[8])/log10(divs[8]*planning_scale/first_line)*log10(dist/first_line);
}
/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "planning.h"
#include "../graphics/component.h"
#include "../monitor.h"
#include <string>
#include "../graphics/icon_button.h"
#include "../window/window.h"
#include "../graphics/display.h"
#include "../graphics/rectangle.h"

void planningConstruct();
window planning_area(planningConstruct);
const int posy[] = {283,250,206,182,164,150,107,64,21};
const int divs[] = {0, 25, 50, 75, 100, 125, 250, 500, 1000};
int planning_scale = 1;
const int object_pos[] = {55,80,105};
Component planning_distance(246,300, displayPlanning);
Component planning_objects(246,300, displayObjects);
Component planning_gradient(18,270, displayGradient);
Component PASP(99,270, displayPASP);
Component planning_speed(99,270, displaySpeed);
bool planning_unchanged= false;
void displayScaleUp();
void displayScaleDown();
void speedLines();
void zoominp()
{
    if(planning_scale>1)
    {
        planning_scale/=2;
        speedLines();
        planning_unchanged = false;
    }
}
void zoomoutp()
{
    if(planning_scale<=16)
    {
        planning_scale*=2;
        speedLines();
        planning_unchanged = false;
    }
}
IconButton zoomin("symbols/Navigation/NA_03.bmp",40,15,zoominp);
IconButton zoomout("symbols/Navigation/NA_04.bmp",40,15,zoomoutp);
vector<planning_element> planning_elements;
void displayPlanning()
{
    for(int i=0; i<9; i++)
    {
        setColor(DarkGrey);
        if(i==0||i==5||i==8) planning_distance.drawRectangle(40, posy[i], 200, 2, MediumGrey);
        else planning_distance.drawLine(40, posy[i], 240-1, posy[i]);
    }
}
void displayObjects()
{
    for(int i = 0; i < planning_elements.size(); i++)
    {
        planning_element p = planning_elements[i];
        if(p.distance>divs[8]*planning_scale || p.distance<0) continue;
        string name = string("symbols/Planning/PL_") + (p.condition < 10 ? "0" : "") + to_string(p.condition)+".bmp";
        planning_distance.drawImage(name.c_str(),object_pos[i%3],getPlanningHeight(p.distance)-25,20,20);
    }
}
vector<gradient_element> gradient_elements;
void displayGradient()
{
    for(int i=0; i+1<gradient_elements.size(); i++)
    {
        gradient_element &e = gradient_elements[i];
        if(e.distance>divs[8]*planning_scale || e.distance<0) continue;
        float max = gradient_elements[i+1].distance;
        float minp = getPlanningHeight(max)-15;
        float maxp = getPlanningHeight(e.distance)-15;
        if(max>divs[8]*planning_scale) minp = 0;
        float size = maxp-minp;
        planning_gradient.drawRectangle(0, minp, 18, size, e.val>=0 ? Grey : DarkGrey);
        planning_gradient.drawLine(0, minp, 17, minp, e.val>=0 ? White : Grey);
        planning_gradient.drawLine(0, minp, 0, maxp-1, e.val>=0 ? White : Grey);
        planning_gradient.drawLine(0, maxp-1, 17, maxp-1, Black);
        if(size>44)
        {
            planning_gradient.drawText(to_string(abs(e.val)).c_str(), 0, (minp+maxp-1)/2-planning_gradient.sy/2, 10, e.val>=0 ? Black : White);
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, minp-planning_gradient.sy/2+7, 10, e.val>=0 ? Black : White);
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, maxp-planning_gradient.sy/2-8, 10, e.val>=0 ? Black : White);
        }
        else if(size>14)
        {
            planning_gradient.drawText(e.val<0 ? "-" : "+", 0, (minp+maxp-1)/2-planning_gradient.sy/2, 10, e.val>=0 ? Black : White);
        }
    }
}
vector<speed_element> speed_elements;
bool check_spdov(int i, int j)
{
    speed_element &cur = speed_elements[i];
    speed_element &chk = speed_elements[j];
    float a = getPlanningHeight(cur.distance)-15;
    float b = getPlanningHeight(chk.distance)-15;
    if(abs(a-b)>18) return false;
    return chk==imarker.element || (cur!=imarker.element && cur.speed>chk.speed);
}
indication_marker imarker;
void displayPASP()
{
    PASP.clear();
    PASP.drawRectangle(14, 0, 99, 270, PASPdark);
    
    if(speed_elements.size() == 0) return;
    speed_element prev_pasp = speed_elements[0];
    bool oth1 = false;
    bool oth2 = false;
    float red = 1;
    float spd = prev_pasp.speed;
    bool end = false;
    for(int i=1; i<speed_elements.size(); i++)
    {
        speed_element cur = speed_elements[i];
        speed_element prev = speed_elements[i-1];
        if (cur.distance < 0) continue;
        if(cur.distance>divs[8]*planning_scale)
        {
            PASP.drawRectangle(14, 0, 93*red, getPlanningHeight(prev_pasp.distance)-15, PASPlight);
            end = true;
            break;
        }
        if(prev_pasp.speed>cur.speed && (!oth2||cur.speed==0))
        {
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
        if(oth1 && prev.speed<cur.speed) oth2 = true;
    }
    if(imarker.start_distance>0) PASP.drawRectangle(14, getPlanningHeight(imarker.start_distance)-15, 93, 2, Yellow);
}
void displaySpeed()
{
    if(planning_unchanged) return;
    planning_unchanged = true;
    planning_speed.clear();
    int ld = 0;
    for(int i=1; i<speed_elements.size(); i++)
    {
        bool ov=false;
        for (int j=1; j<speed_elements.size(); j++)
        {
            if (i!=j && check_spdov(i, j))
            {
                ov = true;
                break;
            }
        }
        if (ov) continue;
        speed_element cur = speed_elements[i];
        speed_element prev = speed_elements[ld];
        if (cur.distance < 0) continue;
        ld = i;
        bool im = imarker.start_distance>0 && (cur==imarker.element);
        if(cur.distance>divs[8]*planning_scale) break;
        float a = getPlanningHeight(cur.distance)-15;
        if(im || prev.speed>cur.speed || cur.speed == 0)
        {
            planning_speed.addImage(im ? "symbols/Planning/PL_23.bmp" : "symbols/Planning/PL_22.bmp", 14, a+7, 20, 20);
            planning_speed.addText(to_string(cur.speed), 25, a-2, 10, im ? Yellow : Grey, UP | LEFT);
        }
        else
        {
            planning_speed.addImage("symbols/Planning/PL_21.bmp", 14, a-7, 20, 20);
            planning_speed.addText(to_string(cur.speed), 25, 270-a-2, 10, Grey, DOWN | LEFT);
        }
        if (cur.speed == 0) return;
    }
}
void speedLines()
{
    planning_distance.clear();
    for(int i=0; i<9; i++)
    {
        if(i==0||i>4)
        {
            planning_distance.addText(to_string(divs[i]*planning_scale), 208, posy[i]-150, 10, White, RIGHT);
        }
    }
}
void planningConstruct()
{
    planning_area.addToLayout(&planning_distance, new RelativeAlignment(nullptr, 334,15));
    planning_area.addToLayout(&planning_objects, new RelativeAlignment(nullptr, 334,15, 0));
    planning_area.addToLayout(&zoomout, new RelativeAlignment(&planning_distance, 20,8,0));
    planning_area.addToLayout(&zoomin, new RelativeAlignment(&planning_distance, 20,292,0));
    planning_area.addToLayout(&planning_gradient, new RelativeAlignment(&planning_distance, 115+9, 15+135, 0));
    planning_area.addToLayout(&PASP, new ConsecutiveAlignment(&planning_gradient, RIGHT, 0));
    planning_area.addToLayout(&planning_speed, new ConsecutiveAlignment(&planning_gradient, RIGHT, 0));
    planning_area.bringFront(&zoomin);
    planning_area.bringFront(&zoomout);
    planning_area.bringFront(&PASP);
    planning_area.bringFront(&planning_distance);
    planning_area.bringFront(&planning_objects);
    planning_area.bringFront(&planning_gradient);
    planning_area.bringFront(&planning_speed);
    zoomin.showBorder = false;
    zoomout.showBorder = false;
    zoomout.touch_down = 15;
    zoomin.touch_up = 15;
    /*planning_elements.push_back({1,500});
    planning_elements.push_back({3,1000});
    planning_elements.push_back({32,930});*/
    speedLines();
}
float getPlanningHeight(float dist)
{
    float first_line = divs[1]*planning_scale;
    if(dist<first_line) return posy[0]-((posy[0]-posy[1])/first_line)*dist;
    else return posy[1]-(posy[1]-posy[8])/log10(divs[8]*planning_scale/first_line)*log10(dist/first_line);
}

/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "planning.h"
#include "../graphics/component.h"
#include "../monitor.h"
#include <string>
#include <cmath>
#include "../graphics/icon_button.h"
#include "../window/window.h"
#include "../graphics/display.h"
#include "../graphics/rectangle.h"

#define METERS_TO_MILES 0.000621371192
#define METERS_TO_FEET 3.2808399
#define KMH_TO_MPH 0.621371192
const int posy[] = {283,250,206,182,164,150,107,64,21};
const int divs[] = { 0, 25, 50, 75, 100, 125, 250, 500, 1000 };
int planning_scale = 1;
const int object_pos[] = {55,80,105};
Component planning_distance(246,300, displayPlanning);
Component planning_objects(246,300, displayObjects);
Component planning_gradient(18,270, displayGradient);
Component PASP(99,270, displayPASP);
Component planning_speed(99,270, displaySpeed);
void zoominp();
void zoomoutp();
IconButton zoomin("symbols/Navigation/NA_03.bmp",40,15,zoominp);
IconButton zoomout("symbols/Navigation/NA_04.bmp",40,15,zoomoutp);
IconButton softzoomin("symbols/Navigation/NA_07.bmp",64,50,zoominp,"symbols/Navigation/NA_09.bmp");
IconButton softzoomout("symbols/Navigation/NA_08.bmp",64,50,zoomoutp,"symbols/Navigation/NA_10.bmp");
extern bool showSpeeds;
extern bool useImperialSystem;
extern bool prevUseImperialSystem;
void displayScaleUp();
void displayScaleDown();
void speedLines();
#if SIMRAIL
#define MAX_SCALE 16
#else
#define MAX_SCALE 32
#endif
void zoominp()
{
    if(planning_scale>1)
    {
        planning_scale/=2;
        speedLines();
    }
    zoomin.setEnabled(planning_scale > 1);
    softzoomin.setEnabled(planning_scale > 1);
    zoomout.setEnabled(planning_scale < MAX_SCALE);
    softzoomout.setEnabled(planning_scale < MAX_SCALE);
}
void zoomoutp()
{
    if(planning_scale<MAX_SCALE)
    {
        planning_scale*=2;
        speedLines();
    }
    zoomin.setEnabled(planning_scale > 1);
    softzoomin.setEnabled(planning_scale > 1);
    zoomout.setEnabled(planning_scale < MAX_SCALE);
    softzoomout.setEnabled(planning_scale < MAX_SCALE);
}
std::vector<planning_element> planning_elements;
void displayPlanning()
{
    for(int i=0; i<9; i++)
    {
        if(i==0||i==5||i==8) planning_distance.drawRectangle(40, posy[i], 200, 2, MediumGrey);
        else planning_distance.drawRectangle(40, posy[i], 200, 1, DarkGrey);
    }
}
std::map<int,std::shared_ptr<UiPlatform::Image>> object_textures;
void displayObjects()
{
    if (prevUseImperialSystem != useImperialSystem) {
        speedLines();
    }

    for(int i = 0; i < planning_elements.size(); i++)
    {
        planning_element p = planning_elements[i];
        if(p.distance>divs[8]*planning_scale || p.distance<0 || (i>2 && getPlanningHeight(planning_elements[i-3].distance)-getPlanningHeight(p.distance) < 20)) continue;
        if (object_textures.find(p.condition) == object_textures.end()) {
            std::string name = std::string("symbols/Planning/PL_") + (p.condition < 10 ? "0" : "") + std::to_string(p.condition)+".bmp";
            object_textures[p.condition] = Component::getImageGraphic(name);
        }
        planning_distance.drawTexture(object_textures[p.condition],object_pos[i%3],getPlanningHeight(p.distance)-5);
    }
}
std::vector<gradient_element> gradient_elements;
/*std::shared_ptr<UiPlatform::Image> plus_white;
std::shared_ptr<UiPlatform::Image> plus_black;
std::shared_ptr<UiPlatform::Image> minus_white;
std::shared_ptr<UiPlatform::Image> minus_black;*/
void displayGradient()
{
    /*if (plus_white == nullptr)
    {
        plus_white = Component::getTextGraphic("+", 10, White, 0);
        plus_black = Component::getTextGraphic("+", 10, Black, 0);
        minus_white = Component::getTextGraphic("-", 10, White, 0);
        minus_black = Component::getTextGraphic("-", 10, Black, 0);
    }*/
    if (mode == Mode::OS && !showSpeeds) return;
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
        planning_gradient.drawRectangle(0, minp, 18, 1, e.val>=0 ? White : Grey);
        planning_gradient.drawRectangle(0, minp, 1, maxp-minp-1, e.val>=0 ? White : Grey);
        planning_gradient.drawRectangle(0, maxp-1, 18, 1, Black);
        if(size>44)
        {
            planning_gradient.draw(planning_gradient.getTextUnique(std::to_string(abs(e.val)).c_str(), 0, (minp+maxp-1)/2-planning_gradient.sy/2, 10, e.val>=0 ? Black : White).get());
            planning_gradient.draw(planning_gradient.getTextUnique(e.val<0 ? "-" : "+", 0, minp-planning_gradient.sy/2+7, 10, e.val>=0 ? Black : White).get());
            planning_gradient.draw(planning_gradient.getTextUnique(e.val<0 ? "-" : "+", 0, maxp-planning_gradient.sy/2-8, 10, e.val>=0 ? Black : White).get());
        }
        else if(size>14)
        {
            planning_gradient.draw(planning_gradient.getTextUnique(e.val<0 ? "-" : "+", 0, (minp+maxp-1)/2-planning_gradient.sy/2, 10, e.val>=0 ? Black : White).get());
        }
    }
}
std::vector<speed_element> speed_elements;
bool check_spdov(int i, int j)
{
    speed_element &cur = speed_elements[i];
    speed_element &chk = speed_elements[j];
    float a = getPlanningHeight(cur.distance)-15;
    float b = getPlanningHeight(chk.distance)-15;
    if(abs(a-b)>18) return false;
    return chk==imarker.element || (cur!=imarker.element && (cur.speed>chk.speed || (cur.speed == chk.speed && cur.distance > chk.distance)));
}
indication_marker imarker;
void displayPASP()
{
    PASP.clear();
    PASP.drawRectangle(14, 0, 99, 270, PASPdark);
    
    if (mode == Mode::OS && !showSpeeds) return;
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
    if(imarker.start_distance>0 && imarker.start_distance <= divs[8]*planning_scale) PASP.drawRectangle(14, getPlanningHeight(imarker.start_distance)-15, 93, 2, Yellow);
}
std::shared_ptr<UiPlatform::Image> pl21;
std::shared_ptr<UiPlatform::Image> pl22;
std::shared_ptr<UiPlatform::Image> pl23;
//std::map<int, std::shared_ptr<UiPlatform::Image>> plspeeds;
void displaySpeed()
{
    if (pl21 == nullptr)
    {
        pl21 = Component::getImageGraphic("symbols/Planning/PL_21.bmp");
        pl22 = Component::getImageGraphic("symbols/Planning/PL_22.bmp");
        pl23 = Component::getImageGraphic("symbols/Planning/PL_23.bmp");
    }
    if (mode == Mode::OS && !showSpeeds) return;
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
            planning_speed.drawTexture(im ? pl23 : pl22, 14, a+7);
            planning_speed.draw(planning_speed.getTextUnique(std::to_string((int)(useImperialSystem ? cur.speed * KMH_TO_MPH : cur.speed)), 25, a-2, 10, im ? Yellow : Grey, UP | LEFT).get());
        }
        else if (prev.speed != cur.speed)
        {
            planning_speed.drawTexture(pl21, 14, a-7);
            planning_speed.draw(planning_speed.getTextUnique(std::to_string((int)(useImperialSystem ? cur.speed * KMH_TO_MPH : cur.speed)), 25, 270-a-2, 10, Grey, DOWN | LEFT).get());
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
            int dist = divs[i] * planning_scale;
            if (useImperialSystem) {
                std::string unit = "";
                if(dist >= 1609.344) {
                    dist = round(dist * METERS_TO_MILES);
                    unit = " mi";
                } else {
                    dist = (((int)(round(dist * METERS_TO_FEET))) / 100) * 100;
                    unit = " ft";
                }
                planning_distance.addText(std::to_string(dist) + unit, 208, posy[i] - 150, 9, White, RIGHT);
            }
            else {
                planning_distance.addText(std::to_string(dist), 208, posy[i] - 150, 9, White, RIGHT);
            }
        }
    }
}
void planningConstruct(window *w)
{
    w->addToLayout(&planning_distance, new RelativeAlignment(nullptr, 334,15));
    w->addToLayout(&planning_objects, new RelativeAlignment(nullptr, 334,15, 0));
    w->addToLayout(&zoomout, new RelativeAlignment(&planning_distance, 20,8,0));
    w->addToLayout(&zoomin, new RelativeAlignment(&planning_distance, 20,292,0));
    w->addToLayout(&planning_gradient, new RelativeAlignment(&planning_distance, 115+9, 15+135, 0));
    w->addToLayout(&PASP, new ConsecutiveAlignment(&planning_gradient, RIGHT, 0));
    w->addToLayout(&planning_speed, new ConsecutiveAlignment(&planning_gradient, RIGHT, 0));
    w->bringFront(&PASP);
    w->bringFront(&planning_distance);
    w->bringFront(&planning_objects);
    w->bringFront(&planning_gradient);
    w->bringFront(&planning_speed);
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
void setPlanning(bool visible)
{
    planning_distance.visible = visible;
    planning_objects.visible = visible;
    planning_gradient.visible = visible;
    PASP.visible = visible;
    planning_speed.visible = visible;
    zoomin.visible = visible;
    zoomout.visible = visible;
}
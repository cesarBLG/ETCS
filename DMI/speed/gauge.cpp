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
#include "../graphics/component.h"
#include "../graphics/texture.h"
#include "../graphics/color.h"
#include "../sound/sound.h"
#include "../STM/stm_objects.h"
#include "gauge.h"
#include "../monitor.h"
#include <string>
#include <cmath>
#include <cstring>
#include <cstdio>
using namespace std;
#define PI 3.14159265358979323846264338327950288419716939937510
int etcsDialMaxSpeed = 400;
int maxSpeed;
const float ang00 = -239*PI/180.0;
const float ang0 = -234*PI/180.0;
const float amed = -42*PI/180.0;
const float ang1 = 54*PI/180.0;
const float cx = 140;
const float cy = 150;
void displaya1();
Component a1(54,54, displaya1);
Component csg(2*cx, 2*cy, displayGauge);
#include "../graphics/text_graphic.h"
text_graphic *spd_nums[10] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
float speedToAngle(float speed)
{
    if(speed>maxSpeed) return ang1;
    if(maxSpeed == 400)
    {
        if(speed>200) return amed + (speed-200)/(maxSpeed-200)*(ang1-amed);
        return ang0 + speed/200*(amed-ang0);
    }
    return ang0 + speed/maxSpeed*(ang1-ang0);
}
void drawNeedle()
{
    Color needleColor = Grey;
    if(mode == Mode::SB)
    {
        needleColor = Grey;
    }
    else if(mode == Mode::SN)
    {
        if (active_ntc_window != nullptr) needleColor = active_ntc_window->monitoring_data.needle_color;
    }
    else if(supervision == IntS)
    {
        needleColor = Red;
    }
    else
    {        
        if(monitoring == RSM && Vest<=Vrelease && Vrelease!=0) needleColor = Yellow;
        if(Vtarget<=Vest && Vest<=Vperm && Vtarget<Vperm && Vtarget>=0)
        {
            if(monitoring==TSM) needleColor = Yellow;
            if(monitoring==CSM) needleColor = White;
        }
        if((Vest>Vperm && monitoring!=RSM) || (Vest>Vrelease && monitoring==RSM))
        {
            needleColor = Orange;
        }
    }
    Color speedColor = needleColor == Red ? White : Black;
    float an = speedToAngle(Vest);
    setColor(needleColor);
    csg.drawCircle(25,cx,cy);
    float px[] = {-4.5, 4.5, 4.5, 1.5, 1.5, -1.5, -1.5, -4.5};
    float py[] = {-15,-15,-82,-90,-105,-105,-90,-82};
    csg.rotateVertex(px, py, 8, cx, cy, an);
    csg.drawPolygon(px, py, 8);

    if(spd_nums[0]==nullptr || spd_nums[0]->color!=speedColor)
    {
        for(int i=0; i<10; i++)
        {
            if(spd_nums[i]!=nullptr) delete spd_nums[i];
            spd_nums[i] = csg.getText(to_string(i), 0, 0, 18, speedColor, RIGHT);
            //spd_nums[i]->load();
        }
    }
    int spd = Vest;
    spd = Vest-spd > 0.01 ? spd + 1 : spd;
    int c[3] = {spd/100%10, spd/10%10, (spd%10)};
    bool firstPrint = false;
    for(int i=0; i<3; i++)
    {
        if(c[i] == 0 && !firstPrint && i!=2) continue;
        firstPrint = true;
        texture *t = spd_nums[c[i]];
        float adj = (i-1) * 15;
        t->x = cx + adj;
        csg.draw(t);
    }
}
void drawHook(float speed)
{
    float ang1 = speedToAngle(speed);
    float ang0 = ang1-6/117.0;
    csg.drawSolidArc(ang0,ang1,117,137,cx,cy);
}
void drawGauge(float minspeed, float maxspeed, Color color, float rmin)
{
    setColor(color);
    float ang0 = speedToAngle(minspeed);
    float ang1 = speedToAngle(maxspeed);
    csg.drawSolidArc(ang0,ang1,rmin,137,cx,cy);
}
void drawGauge(float minspeed, float maxspeed, Color color)
{
    drawGauge(minspeed,maxspeed,color,128);
}
void drawSetSpeed()
{
    if (Vset == 0) return;
    float an = speedToAngle(Vset);
    setColor(White);
    csg.drawCircle(5, 111*cos(an) + cx, 111*sin(an) + cy);
}
bool showSpeeds = false;
void displayCSG()
{
    if (mode == Mode::OS || mode == Mode::SR || mode == Mode::SH)
    {
        csg.setPressedAction([]() {showSpeeds = !showSpeeds;});
    }
    else
    {
        csg.setPressedAction(nullptr);
    }
    if (mode == Mode::SN)
    {
        if (active_ntc_window == nullptr) return;
        stm_monitoring_data stm = active_ntc_window->monitoring_data;
        if (stm.Vtarget_display & 2 || stm.Vperm_display & 2 || stm.Vsbi_display & 2 || stm.Vrelease_display & 2)
        {
            setColor(DarkGrey);
            csg.drawSolidArc(ang00, ang0, 128, 137, cx, cy);
        }
        setColor(stm.Vsbi_color);
        if (stm.Vsbi_display) drawGauge(Vperm, Vsbi, stm.Vsbi_color, stm.Vsbi_display == 2 ? 117 : 128);
        setColor(stm.Vtarget_color);
        if (stm.Vtarget_display & 2) drawGauge(stm.Vperm_display && Vperm < Vtarget ? Vperm : 0, Vtarget, stm.Vtarget_color);
        if (stm.Vtarget_display & 1) drawHook(Vtarget);
        setColor(stm.Vperm_color);
        if (stm.Vperm_display & 2) drawGauge(stm.Vtarget_display && Vperm >= Vtarget ? Vtarget : 0, Vperm, stm.Vperm_color);
        if (stm.Vperm_display & 1) drawHook(Vperm);
        if (stm.Vrelease_display & 2) drawGauge(0,Vrelease,stm.Vrelease_color,133);
        return;
    }
    if(mode != Mode::FS)
    {
        if((mode == Mode::OS || mode == Mode::SR) && showSpeeds)
        {
            setColor(White);
            drawHook(Vperm);
            if(monitoring != CSM || (Vtarget<Vperm && Vtarget>=0))
            {
                setColor(MediumGrey);
                drawHook(Vtarget);
            }
        }
        if((mode == Mode::SH && showSpeeds) || mode == Mode::RV)
        {
            setColor(White);
            drawHook(Vperm);
        }
        return;
    } 
    setColor(DarkGrey);
    csg.drawSolidArc(ang00, ang0, 128, 137, cx, cy);
    if(monitoring == CSM)
    {
        if(Vtarget<Vperm && Vtarget>=0)
        {
            drawGauge(0,Vtarget, DarkGrey);
            drawGauge(Vtarget, Vperm, White);
        }
        else drawGauge(0,Vperm, DarkGrey);
        drawHook(Vperm);
    }
    if(monitoring == TSM)
    {
        drawGauge(0,Vtarget, DarkGrey);
        drawGauge(Vtarget, Vperm, Yellow);
        drawHook(Vperm);
    }
    if(monitoring == RSM)
    {
        setColor(Yellow);
        drawHook(Vperm);
    }
    if(supervision == OvS || supervision == WaS) drawGauge(Vperm,Vsbi,Orange,117);
    if(supervision == IntS) drawGauge(Vperm,Vsbi,Red,117);
    if(Vrelease!=0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM))
    {
        if(Vperm<Vrelease)
        {
            drawGauge(0, Vrelease, MediumGrey);
            float ang = speedToAngle(Vperm);
            setColor(Black);
            csg.drawSolidArc(ang0,ang,132,133,cx,cy);
            setColor(Yellow);
            csg.drawSolidArc(ang0,ang,128, 132,cx,cy);
        }
        else
        {
            if(monitoring == RSM) drawGauge(0, Vperm, Yellow);
            drawGauge(0,Vrelease,Black,132);
            drawGauge(0,Vrelease,MediumGrey,133);
        }
    }
}
static int initSpeed = 0;
void displayLines()
{
    bool inited = initSpeed != maxSpeed;
    initSpeed = maxSpeed;
    if (!inited) csg.clear();
    setColor(White);
    int step = maxSpeed == 150 ? 5 : 10;
    for(int i = 0; i<=maxSpeed; i+=step)
    {
        float size;
        float an = speedToAngle(i);
        int longinterval = maxSpeed == 400 ? 50 : (maxSpeed == 150 ? 25 : 20);
        size = i%longinterval!=0 ? -110 : -100;
        if(!inited && i%longinterval == 0 && (maxSpeed != 400 || (i!=250 && i!=350)))
        {
            std::string s = to_string(i);
            const char *str = s.c_str();
            float hx = 0;
            float hy = 12/2.0;
            TTF_Font *font = openFont(fontPath, 16);
            if (font == nullptr) return;
            SDL_Color white = {(Uint8)White.R, (Uint8)White.G, (Uint8)White.B};
            float width;
            float height;
            getFontSize(font, str, &width, &height);
            hx = width/2 + 1;
            hy = TTF_FontAscent(font)/getScale(1)/2 - 2;
            float maxan = atanf(hy/hx);
            float cuadran = abs(-an-PI/2);
            float adjust = (abs(PI/2-cuadran) > maxan) ? hy/abs(cosf(cuadran)) : hx/sinf(cuadran);
            float val = size + adjust;
            SDL_Surface *surf = TTF_RenderText_Blended(font, str, white);
            //TTF_CloseFont(font);
            texture *t = new texture();
            t->x = cx-val*cosf(an);
            t->y = cy-val*sinf(an);
            t->width = width;
            t->height = height;
            t->tex = std::shared_ptr<sdl_texture>(new sdl_texture(SDL_CreateTextureFromSurface(sdlren, surf)));
            csg.add(t);
        }
        csg.drawRadius(cx, cy, size, -125, an);
    }
}
Component releaseRegion(36,36, displayVrelease);
static float prevVrelease=0;
bool releaseSignShown=false;
void displayVrelease()
{
    if (mode == Mode::SN && active_ntc_window != nullptr && active_ntc_window->monitoring_data.Vrelease_display & 1) {
        if(prevVrelease!=Vrelease || !releaseSignShown)
        {
            releaseSignShown = true;
            releaseRegion.clear();
            releaseRegion.addText(to_string((int)std::round(Vrelease)).c_str(), 0, 0, 17, active_ntc_window->monitoring_data.Vrelease_color, CENTER, 0);
            prevVrelease = Vrelease;
        }
    } else if (Vrelease!=0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM) && (mode != Mode::OS || showSpeeds) && mode != Mode::SN) {
        if(prevVrelease!=Vrelease || !releaseSignShown)
        {
            releaseSignShown = true;
            releaseRegion.clear();
            releaseRegion.addText(to_string((int)std::round(Vrelease)).c_str(), 0, 0, 17, MediumGrey, CENTER, 0);
            prevVrelease = Vrelease;
        }
    } else {
        releaseSignShown = false;
        releaseRegion.clear();
    }
}
void displayGauge()
{
    displayLines();
    displayCSG();
    drawNeedle();
    drawSetSpeed();
}
bool ttiShown = false;
const float TdispTTI = 14;
void displaya1()
{
    if(mode == Mode::LS)
    {
        //a1.drawImage("symbols/Limited Supervision/MO_21.bmp");
        //a1.addText("120", 12, 0, 0, White);
    }
    if((mode == Mode::FS || ((mode == Mode::OS || mode == Mode::SR) && showSpeeds)) && monitoring == CSM && TTI < TdispTTI)
    {
        if(!ttiShown) playSinfo();
        ttiShown = true;
        int n;
        for(n = 1; n<11; n++)
        {
            if(TdispTTI*(10-n)/10<=TTI && TTI<TdispTTI*(10-(n-1))/10) break;
        }
        a1.drawRectangle(0, 0, 50, 50, White, CENTER);
        a1.drawRectangle(0, 0, 5*n, 5*n, Grey, CENTER);
    }
    else ttiShown = false;
    if (mode == Mode::FS && (monitoring == TSM || monitoring == RSM) && TTP < TdispTTI) {
        int n;
        for(n = 1; n<11; n++)
        {
            if(TdispTTI*(10-n)/10<=TTP && TTP<TdispTTI*(10-(n-1))/10) break;
        }
        a1.drawRectangle(0, 0, 5*n, 5*n, supervision == IntS ? Red : ((supervision == WaS || supervision == OvS) ? Orange : Yellow), CENTER);
    }
}

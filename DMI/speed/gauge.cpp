/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "../graphics/component.h"
#include "../graphics/icon_button.h"
#include "../graphics/texture.h"
#include "../graphics/color.h"
#include "../sound/sound.h"
#include "../STM/stm_objects.h"
#include "gauge.h"
#include "../monitor.h"
#include <string>
#include <cmath>
#include "platform_runtime.h"
using namespace std;
#define PI 3.14159265358979323846264338327950288419716939937510
#define KMH_TO_MPH 0.621371192
extern bool useImperialSystem;
extern bool prevUseImperialSystem;
int etcsDialMaxSpeed = 400;
int maxSpeed;
const float ang00 = -239*PI/180.0;
const float ang0 = -234*PI/180.0;
const float amed = -42*PI/180.0;
const float ang1 = 54*PI/180.0;
const float cx = 140;
const float cy = 150;
bool showSpeeds = false;
bool displayTTPavailable = true;
void displaya1();
Component a1(54,54, displaya1);
Component csg(2*cx, 2*cy, displayGauge);
#if BASELINE == 4
Component b8(36,36);
#endif
IconButton togglingButton("symbols/Driver Request/DR_01.bmp", 64, 50, []() {showSpeeds = !showSpeeds;});
#include "../graphics/text_graphic.h"
text_graphic *spd_nums[10] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
std::shared_ptr<UiPlatform::Image> mphIndicator;

#if SIMRAIL
float speedToAngle(float speed)
{
    if (speed > maxSpeed) return ang1;
    int halfEtcsDialMaxSpeed = maxSpeed / 2;
    {
        if (speed > halfEtcsDialMaxSpeed) return amed + (speed - halfEtcsDialMaxSpeed) / (maxSpeed - halfEtcsDialMaxSpeed) * (ang1 - amed);
        return ang0 + speed / halfEtcsDialMaxSpeed * (amed - ang0);
    }
    return ang0 + speed / maxSpeed * (ang1 - ang0);
}
#else
float speedToAngle(float speed)
{
    if (speed > maxSpeed) return ang1;
    if (maxSpeed == 400)
    {
        if (speed > 200) return amed + (speed - 200) / (maxSpeed - 200) * (ang1 - amed);
        return ang0 + speed / 200 * (amed - ang0);
    }
    return ang0 + speed / maxSpeed * (ang1 - ang0);
}
#endif

void drawNeedle()
{
    Color needleColor = Grey;
    if(mode == Mode::SB || mode == Mode::NL || mode == Mode::PT || mode == Mode::IS)
    {
        needleColor = Grey;
    }
    else if(mode == Mode::SN)
    {
        if (active_ntc_window != nullptr) needleColor = active_ntc_window->monitoring_data.needle_color;
    }
#if BASELINE == 4
    else if (mode == Mode::AD)
    {
        needleColor = Vtarget<=Vest || monitoring == RSM ? White : Grey;
    }
#endif
    else if(supervision == IntS)
    {
        needleColor = Red;
    }
    else
    {
        if(monitoring == RSM && Vest<=Vrelease && Vrelease!=0) needleColor = Yellow;
        if(Vtarget<=Vest && Vest<=Vperm && Vtarget<Vperm && Vtarget>=0 && mode != Mode::LS)
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
    float an = speedToAngle(useImperialSystem ? Vest * KMH_TO_MPH : Vest);
    platform->set_color(needleColor);
    csg.drawCircle(25,cx,cy);
    float pax[] = { 1.5f, 1.5f, -1.5f, -1.5f };
    float pay[] = { -90.0f, -105.0f, -105.0f, -90.0f };
    float pbx[] = { -4.5f, 4.5f, 4.5f, 1.5f, -1.5f, -4.5f };
    float pby[] = { -15.0f, -15.0f, -82.0f, -90.0f, -90.0f, -82.0f };
    csg.rotateVertex(pax, pay, 4, cx, cy, an);
    csg.rotateVertex(pbx, pby, 6, cx, cy, an);
    csg.drawConvexPolygon(pax, pay, 4);
    csg.drawConvexPolygon(pbx, pby, 6);
    if(spd_nums[0]==nullptr || spd_nums[0]->color!=speedColor)
    {
        for(int i=0; i<10; i++)
        {
            if(spd_nums[i]!=nullptr) delete spd_nums[i];
            spd_nums[i] = csg.getText(to_string(i), 0, 0, 18, speedColor, RIGHT);
        }
    }
    int spd = useImperialSystem ? Vest * KMH_TO_MPH : Vest;
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
    float ang1 = speedToAngle(useImperialSystem ? speed * KMH_TO_MPH : speed);
    float ang0 = ang1-6/117.0;
    csg.drawSolidArc(ang0,ang1,117,137,cx,cy);
}
void drawGauge(float minspeed, float maxspeed, Color color, float rmin)
{
    platform->set_color(color);
    float ang0 = speedToAngle(useImperialSystem ? minspeed * KMH_TO_MPH : minspeed);
	float ang1 = speedToAngle(useImperialSystem ? maxspeed * KMH_TO_MPH : maxspeed);
    csg.drawSolidArc(ang0,ang1,rmin,137,cx,cy);
}
void drawGauge(float minspeed, float maxspeed, Color color)
{
    drawGauge(minspeed,maxspeed,color,128);
}
void drawImperialIndicator()
{
    if (!useImperialSystem)
        return;

    if (mphIndicator == nullptr) {
        string s = "mph";
        std::unique_ptr<UiPlatform::Font> mphFont = platform->load_font(12, false, "");
        mphIndicator = platform->make_text_image(s, *mphFont, White);
    }
    csg.drawTexture(mphIndicator, 140, 230);
}
void drawSetSpeed()
{
    if (Vset < 0) return;
    float an = speedToAngle(useImperialSystem ? Vset * KMH_TO_MPH : Vset);

    platform->set_color(White);
    csg.drawCircle(5, 111 * cos(an) + cx, 111 * sin(an) + cy);
}
static Mode prevmode;
void displayCSG()
{
    if (mode != prevmode) showSpeeds = false;
    prevmode = mode;
    if (mode == Mode::OS || mode == Mode::SR || mode == Mode::SH)
    {
        csg.setPressedAction([]() {showSpeeds = !showSpeeds;});
        togglingButton.visible = true;
    }
    else
    {
        csg.setPressedAction(nullptr);
        togglingButton.visible = false;
    }
    if (mode == Mode::SN)
    {
        if (active_ntc_window == nullptr) return;
        stm_monitoring_data stm = active_ntc_window->monitoring_data;
        if (stm.Vtarget_display & 2 || stm.Vperm_display & 2 || stm.Vsbi_display & 2 || stm.Vrelease_display & 2)
        {
            platform->set_color(DarkGrey);
            csg.drawSolidArc(ang00, ang0, 128, 137, cx, cy);
        }
        platform->set_color(stm.Vsbi_color);
        if (stm.Vsbi_display) drawGauge(Vperm, Vsbi, stm.Vsbi_color, stm.Vsbi_display == 2 ? 117 : 128);
        platform->set_color(stm.Vtarget_color);
        if (stm.Vtarget_display & 2) drawGauge(stm.Vperm_display && Vperm < Vtarget ? Vperm : 0, Vtarget, stm.Vtarget_color);
        if (stm.Vtarget_display & 1) drawHook(Vtarget);
        platform->set_color(stm.Vperm_color);
        if (stm.Vperm_display & 2) drawGauge(stm.Vtarget_display && Vperm >= Vtarget ? Vtarget : 0, Vperm, stm.Vperm_color);
        if (stm.Vperm_display & 1) drawHook(Vperm);
        if (stm.Vrelease_display & 2) drawGauge(0,Vrelease,stm.Vrelease_color,133);
        return;
    }
#if BASELINE < 4
    if (mode != Mode::FS)
#else
    if (mode != Mode::FS && mode != Mode::AD)
#endif
    {
        if((mode == Mode::OS || mode == Mode::SR) && showSpeeds
#if BASELINE == 4
        || mode == Mode::SM
#endif
        )
        {
            platform->set_color(White);
            drawHook(Vperm);
            if(monitoring != CSM || (Vtarget<Vperm && Vtarget>=0))
            {
                platform->set_color(MediumGrey);
                drawHook(Vtarget);
            }
        }
        if((mode == Mode::SH && showSpeeds) || mode == Mode::RV)
        {
            platform->set_color(White);
            drawHook(Vperm);
        }
        return;
    } 
    platform->set_color(DarkGrey);
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
#if BASELINE == 4
        drawGauge(Vtarget, Vperm, mode == Mode::AD ? White : Yellow);
#else
        drawGauge(Vtarget, Vperm, Yellow);
#endif
        drawHook(Vperm);
    }
    if(monitoring == RSM)
    {
#if BASELINE == 4
        platform->set_color(mode == Mode::AD ? White : Yellow);
#else
        platform->set_color(Yellow);
#endif
        drawHook(Vperm);
    }
    if(supervision == OvS || supervision == WaS)
    {
#if BASELINE == 4
        drawGauge(Vperm,Vsbi, mode == Mode::AD ? (Vtarget<Vperm ? White : DarkGrey) : Orange,117);
#else
        drawGauge(Vperm,Vsbi,Orange,117);
#endif
    }
    if(supervision == IntS) drawGauge(Vperm,Vsbi,Red,117);
    if(Vrelease!=0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM))
    {
        if(Vperm<Vrelease)
        {
            drawGauge(0, Vrelease, MediumGrey);
            float ang = speedToAngle(Vperm);
            platform->set_color(Black);
            csg.drawSolidArc(ang0,ang,132,133,cx,cy);
#if BASELINE == 4
            platform->set_color(mode == Mode::AD ? White : Yellow);
#else
            platform->set_color(Yellow);
#endif
            csg.drawSolidArc(ang0,ang,128, 132,cx,cy);
        }
        else
        {
            if(monitoring == RSM)
            {
#if BASELINE == 4
                drawGauge(0, Vperm, mode == Mode::AD ? White : Yellow);
#else
                drawGauge(0, Vperm, Yellow);
#endif
            }
            drawGauge(0,Vrelease,Black,132);
            drawGauge(0,Vrelease,MediumGrey,133);
        }
    }
}
int prevMaxSpeed = 0;
void displayLines(bool redraw)
{
    std::unique_ptr<UiPlatform::Font> gaugeFont;
    if (redraw)
        gaugeFont = platform->load_font(16, false, "");
    platform->set_color(White);

#if SIMRAIL
    int step = maxSpeed < 160 ? 5 : 10;;
#else
    int step = maxSpeed == 150 ? 5 : 10;
#endif
    for(int i = 0; i<=maxSpeed; i+=step)
    {
        float rminline;
        float an = speedToAngle(i);
#if SIMRAIL
        int longinterval = maxSpeed == 400 ? 50 : (i > maxSpeed / 2 ? 40 : 20);
#else
        int longinterval = maxSpeed == 400 ? 50 : (maxSpeed == 150 ? 25 : 20);
#endif
        rminline = i%longinterval!=0 ? -110 : -100;
        if(redraw && i%longinterval == 0 && (maxSpeed != 400 || (i!=250 && i!=350)))
        {
            std::string s = to_string(i);
            std::pair<float, float> wh = gaugeFont->calc_size(s);
            float hx = wh.first/2;
#if SIMRAIL
            float hy = wh.second/2;
#else
            float hy = 16/2;
#endif
            float textDiagAn = atanf(hy/hx);
            float speedLineAn = abs(-an-PI/2);
            // Depending on the speed line angle, the radius will cross the text rectangle
            // from left to right, or from top to bottom.
            // In either case, compute the half-length of the radius segment inside the rectangle
            float hradiusIn = (abs(PI/2-speedLineAn) > textDiagAn) ? hy/abs(cosf(speedLineAn)) : hx/sinf(speedLineAn);
#if SIMRAIL
            float pad = 5;
#else
            float pad = 2;
#endif
            float rcenter = rminline + hradiusIn + pad;
            texture *t = new texture();
            t->x = cx-rcenter*cosf(an);
            t->y = cy-rcenter*sinf(an);
            t->width = wh.first;
            t->height = wh.second;
            t->tex = platform->make_text_image(s, *gaugeFont, White);
            csg.add(t);
        }
        csg.drawRadius(cx, cy, rminline, -125, an);
    }
}
Component releaseRegion(36,36, displayVrelease);
static float prevVrelease=0;
bool releaseSignShown=false;
void displayVrelease()
{
    if (prevUseImperialSystem != useImperialSystem)
        releaseSignShown = false;

    if (mode == Mode::SN && active_ntc_window != nullptr && active_ntc_window->monitoring_data.Vrelease_display & 1) {
        if(prevVrelease!=Vrelease || !releaseSignShown)
        {
            releaseSignShown = true;
            releaseRegion.clear();
            releaseRegion.addText(to_string((int)std::round(useImperialSystem ? Vrelease * KMH_TO_MPH : Vrelease)).c_str(), 0, 0, 17, active_ntc_window->monitoring_data.Vrelease_color, CENTER, 0);
            prevVrelease = Vrelease;
        }
    } else if (Vrelease!=0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM) && (mode != Mode::OS || showSpeeds) && mode != Mode::SN) {
        if(prevVrelease!=Vrelease || !releaseSignShown)
        {
            releaseSignShown = true;
            releaseRegion.clear();
            releaseRegion.addText(to_string((int)std::round(useImperialSystem ? Vrelease * KMH_TO_MPH : Vrelease)).c_str(), 0, 0, 17,
#if BASELINE == 4
            mode == Mode::AD ? MediumGrey : Yellow,
#else
            MediumGrey,
#endif
            CENTER, 0);
            prevVrelease = Vrelease;
        }
    } else {
        releaseSignShown = false;
        releaseRegion.clear();
    }
}
void displayGauge()
{
	if (prevUseImperialSystem != useImperialSystem) {
		maxSpeed = useImperialSystem ? (((int)(etcsDialMaxSpeed * KMH_TO_MPH / 20) * 20) + 20) : etcsDialMaxSpeed;
		prevMaxSpeed = 0;
		csg.clear();
	}
	
    bool inited = prevMaxSpeed == maxSpeed;
    prevMaxSpeed = maxSpeed;
    std::unique_ptr<UiPlatform::Font> gaugeFont;
    if (!inited) {
        mphIndicator = nullptr;
        for(int i=0; i<10; i++)
        {
            if(spd_nums[i]!=nullptr)
            {
                delete spd_nums[i];
                spd_nums[i] = nullptr;
            }
        }
        csg.clear();
    }

    displayLines(!inited);
    displayCSG();
    drawNeedle();
    drawImperialIndicator();
    drawSetSpeed();
}
bool ttiShown = false;
const float TdispTTI = 14;
int lssma;
void setLSSMA(int nlssma)
{
    if (nlssma != lssma)
    {
        lssma = nlssma;
        a1.clear();
        if (lssma >= 0)
        {
            a1.addImage("symbols/Limited Supervision/MO_21.bmp");
            a1.addText(std::to_string(lssma), 0, 0, 12, White);
        }
    }
}
void displaya1()
{
    if(mode == Mode::LS) return;
    if((mode == Mode::FS
#if BASELINE == 4
    || mode == Mode::AD || mode == Mode::SM
#endif
    || ((mode == Mode::OS || mode == Mode::SR) && showSpeeds)) && monitoring == CSM && TTI < TdispTTI)
    {
        if(!ttiShown)
        {
#if BASELINE == 4
            if (mode != Mode::AD) playSinfo();
#else
            playSinfo();
#endif
        }
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
    if (displayTTPavailable && mode == Mode::FS && (monitoring == TSM || monitoring == RSM) && TTP < TdispTTI) {
        int n;
        for(n = 1; n<11; n++)
        {
            if(TdispTTI*(10-n)/10<=TTP && TTP<TdispTTI*(10-(n-1))/10) break;
        }
        a1.drawRectangle(0, 0, 5*n, 5*n, supervision == IntS ? Red : ((supervision == WaS || supervision == OvS) ? Orange : Yellow), CENTER);
    }
}

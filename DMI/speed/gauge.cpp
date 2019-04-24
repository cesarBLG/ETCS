#include "../graphics/component.h"
#include "../graphics/color.h"
#include "../sound/sound.h"
#include "gauge.h"
#include "../monitor.h"
#include <string>
#include <cmath>
#include <cstring>
#include <cstdio>
using namespace std;
#define PI 3.14159265358979323846264338327950288419716939937510
int maxSpeed = 400;
const float ang00 = -239*PI/180.0;
const float ang0 = -234*PI/180.0;
const float amed = -42*PI/180.0;
const float ang1 = 54*PI/180.0;
const float cx = 140;
const float cy = 150;
void displaya1();
Component a1(54,54, displaya1);
Component csg(2*cx, 2*cy, displayGauge);
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
    Color speedColor = Black;
    if(mode == SB)
    {
        needleColor = Grey;
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
            if(supervision == IntS)
            {
                needleColor = Red;
                speedColor = White;
            }
            else needleColor = Orange;
        }
    }
    float an = speedToAngle(Vest);
    setColor(needleColor);
    csg.drawCircle(25,cx,cy);
    float px[] = {-4.5, 4.5, 4.5, 1.5, 1.5, -1.5, -1.5, -4.5};
    float py[] = {-15,-15,-82,-90,-105,-105,-90,-82};
    short psx[8];
    short psy[8];
    csg.rotateVertex(px, py, 8, cx, cy, an);
    csg.drawPolygon(px, py, 8);
    int spd = Vest;
    spd = Vest-spd > 0.01 ? spd + 1 : spd;
    int c[3] = {spd/100%10 + 48, spd/10%10 + 48, (spd%10) + 48};
    bool firstPrint = false;
    for(int i=0; i<3; i++)
    {
        if(c[i] == '0' && !firstPrint && i!=2) continue;
        firstPrint = true;
        char ch[] = {(int)c[i], 0};
        float adj = -24 + (2-i) * 15;
        csg.drawText(ch, cx + adj, 0, 0, 0, 18, speedColor, RIGHT);
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
void drawSetSpeed(float speed)
{
    float an = speedToAngle(speed);
    setColor(White);
    csg.drawCircle(5, 111*cos(an) + cx, 111*sin(an) + cy);
}
bool showSpeeds = false;
void displayCSG()
{
    if(mode != FS)
    {
        if((mode == OS || mode == SR) && showSpeeds)
        {
            setColor(White);
            drawHook(Vperm);
            if(monitoring != CSM || (Vtarget<Vperm && Vtarget>=0))
            {
                setColor(MediumGrey);
                drawHook(Vtarget);
            }
        }
        if((mode == SH && showSpeeds) || mode == RV)
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
void displayLines()
{
    setColor(White);
    for(int i = 0; i<=maxSpeed; i+=10)
    {
        float size;
        float an = speedToAngle(i);
        int longinterval = maxSpeed == 400 ? 50 : 20;
        size = i%longinterval!=0 ? -110 : -100;
        if((maxSpeed != 400 && i%20==0) || (maxSpeed == 400 && i%50==0 && i!=250 && i!=350))
        {
            const char *str = to_string(i).c_str();
            float hx = 0;
            float hy = 12/2.0;
            TTF_Font *font = openFont(fontPath, 16);
            SDL_Color white = {White.R, White.G, White.B};
            float width;
            float height;
            getFontSize(font, str, &width, &height);
            hx = width/2 + 1;
            hy = TTF_FontAscent(font)/getScale(1)/2 - 2;
            float maxan = atanf(hy/hx);
            float cuadran = abs(-an-PI/2);
            float adjust = (abs(PI/2-cuadran) > maxan) ? hy/abs(cosf(cuadran)) : hx/sinf(cuadran);
            float val = size + adjust;
            csg.drawSurface(TTF_RenderText_Blended(font, str, white), cx-val*cosf(an), cy-val*sinf(an), width, height);
            TTF_CloseFont(font);
        }
        csg.drawRadius(cx, cy, size, -125, an);
    }
}
Component releaseRegion(36,36, displayVrelease);
void displayVrelease()
{
    if(Vrelease!=0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM) && (mode != OS || showSpeeds))
    {
        releaseRegion.setText(to_string((int)Vrelease).c_str(),17, MediumGrey);
    }
}
void displayGauge()
{
    displayLines();
    displayCSG();
    drawNeedle();
    //drawSetSpeed(50);
}
bool ttiShown = false;
const float TdispTTI = 10;
void displaya1()
{
    if(mode == LS)
    {
        a1.setBackgroundImage("symbols/Limited Supervision/MO_21.bmp");
        a1.setText("120", 12, White);
    }
    if((mode == FS || ((mode == OS || mode == SR) && showSpeeds)) && monitoring == CSM && TTI < TdispTTI)
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
}
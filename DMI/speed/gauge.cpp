/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include "platform_runtime.h"
using namespace std;
#define PI 3.14159265358979323846264338327950288419716939937510
#define MPH 0.621371192
bool useImperialSystem;
bool prevUseImperialSystem;
int etcsDialMaxSpeed = 400;
int maxSpeed;
const float ang00 = -239 * PI / 180.0;
const float ang0 = -234 * PI / 180.0;
const float amed = -42 * PI / 180.0;
const float ang1 = 54 * PI / 180.0;
const float cx = 140;
const float cy = 150;
void displaya1();
Component a1(54, 54, displaya1);
Component csg(2 * cx, 2 * cy, displayGauge);
#include "../graphics/text_graphic.h"
text_graphic* spd_nums[10] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
std::shared_ptr<UiPlatform::Image> mphIndicator;

#if SIMRAIL
float speedToAngle(float speed)
{
	if (speed > maxSpeed) return ang1;
	int halfMaxSpeed = maxSpeed / 2;
	{
		if (speed > halfMaxSpeed) return amed + (speed - halfMaxSpeed) / (maxSpeed - halfMaxSpeed) * (ang1 - amed);
		return ang0 + speed / halfMaxSpeed * (amed - ang0);
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
	if (mode == Mode::SB || mode == Mode::NL || mode == Mode::PT || mode == Mode::IS)
	{
		needleColor = Grey;
	}
	else if (mode == Mode::SN)
	{
		if (active_ntc_window != nullptr) needleColor = active_ntc_window->monitoring_data.needle_color;
	}
	else if (supervision == IntS)
	{
		needleColor = Red;
	}
	else
	{
		if (monitoring == RSM && Vest <= Vrelease && Vrelease != 0) needleColor = Yellow;
		if (Vtarget <= Vest && Vest <= Vperm && Vtarget < Vperm && Vtarget >= 0 && mode != Mode::LS)
		{
			if (monitoring == TSM) needleColor = Yellow;
			if (monitoring == CSM) needleColor = White;
		}
		if ((Vest > Vperm && monitoring != RSM) || (Vest > Vrelease && monitoring == RSM))
		{
			needleColor = Orange;
		}
	}
	Color speedColor = needleColor == Red ? White : Black;
	float an = speedToAngle(useImperialSystem ? Vest * MPH : Vest);
	platform->set_color(needleColor);
	csg.drawCircle(25, cx, cy);
	float px[] = { -4.5, 4.5, 4.5, 1.5, 1.5, -1.5, -1.5, -4.5 };
	float py[] = { -15,-15,-82,-90,-105,-105,-90,-82 };
	csg.rotateVertex(px, py, 8, cx, cy, an);
	csg.drawPolygon(px, py, 8);

	if (spd_nums[0] == nullptr || spd_nums[0]->color != speedColor)
	{
		for (int i = 0; i < 10; i++)
		{
			if (spd_nums[i] != nullptr) delete spd_nums[i];
			spd_nums[i] = csg.getText(to_string(i), 0, 0, 18, speedColor, RIGHT);
			//spd_nums[i]->load();
		}
	}
	int spd = useImperialSystem ? Vest * MPH : Vest;
	spd = Vest - spd > 0.01 ? spd + 1 : spd;
	int c[3] = { spd / 100 % 10, spd / 10 % 10, (spd % 10) };
	bool firstPrint = false;
	for (int i = 0; i < 3; i++)
	{
		if (c[i] == 0 && !firstPrint && i != 2) continue;
		firstPrint = true;
		texture* t = spd_nums[c[i]];
		float adj = (i - 1) * 15;
		t->x = cx + adj;
		csg.draw(t);
	}
}
void drawHook(float speed)
{
	float ang1 = speedToAngle(speed);
	float ang0 = ang1 - 6 / 117.0;
	csg.drawSolidArc(ang0, ang1, 117, 137, cx, cy);
}
void drawGauge(float minspeed, float maxspeed, Color color, float rmin)
{
	platform->set_color(color);
	float ang0 = speedToAngle(minspeed);
	float ang1 = speedToAngle(maxspeed);
	csg.drawSolidArc(ang0, ang1, rmin, 137, cx, cy);
}
void drawGauge(float minspeed, float maxspeed, Color color)
{
	drawGauge(minspeed, maxspeed, color, 128);
}
void drawImperialIndicator()
{
	if (!useImperialSystem)
		return;

	if (mphIndicator == NULL) {
		string s = "MPH";
		std::unique_ptr<UiPlatform::Font> mphFont = platform->load_font(13, false);
		mphIndicator = platform->make_text_image(s, *mphFont, White);
	}
	csg.drawTexture(mphIndicator, 140, 230, mphIndicator->width(), mphIndicator->height());
}
void drawSetSpeed()
{
	if (Vset == 0) return;
	float an = speedToAngle(Vset);

	platform->set_color(White);
	csg.drawCircle(4, 121 * cos(an) + cx, 121 * sin(an) + cy);

	platform->set_color(Magenta);
	csg.drawCircle(3, 121 * cos(an) + cx, 121 * sin(an) + cy);

}
bool showSpeeds = false;
void displayCSG()
{
	if (mode == Mode::OS || mode == Mode::SR || mode == Mode::SH)
	{
		csg.setPressedAction([]() {showSpeeds = !showSpeeds; });
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
		if (stm.Vrelease_display & 2) drawGauge(0, Vrelease, stm.Vrelease_color, 133);
		return;
	}
	if (mode != Mode::FS)
	{
		if ((mode == Mode::OS || mode == Mode::SR) && showSpeeds)
		{
			platform->set_color(White);
			drawHook(Vperm);
			if (monitoring != CSM || (Vtarget < Vperm && Vtarget >= 0))
			{
				platform->set_color(MediumGrey);
				drawHook(Vtarget);
			}
		}
		if ((mode == Mode::SH && showSpeeds) || mode == Mode::RV)
		{
			platform->set_color(White);
			drawHook(Vperm);
		}
		return;
	}
	platform->set_color(DarkGrey);
	csg.drawSolidArc(ang00, ang0, 128, 137, cx, cy);
	if (monitoring == CSM)
	{
		if (Vtarget < Vperm && Vtarget >= 0)
		{
			drawGauge(0, Vtarget, DarkGrey);
			drawGauge(Vtarget, Vperm, White);
		}
		else drawGauge(0, Vperm, DarkGrey);
		drawHook(Vperm);
	}
	if (monitoring == TSM)
	{
		drawGauge(0, Vtarget, DarkGrey);
		drawGauge(Vtarget, Vperm, Yellow);
		drawHook(Vperm);
	}
	if (monitoring == RSM)
	{
		platform->set_color(Yellow);
		drawHook(Vperm);
	}
	if (supervision == OvS || supervision == WaS) drawGauge(Vperm, Vsbi, Orange, 117);
	if (supervision == IntS) drawGauge(Vperm, Vsbi, Red, 117);
	if (Vrelease != 0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM))
	{
		if (Vperm < Vrelease)
		{
			drawGauge(0, Vrelease, MediumGrey);
			float ang = speedToAngle(Vperm);
			platform->set_color(Black);
			csg.drawSolidArc(ang0, ang, 132, 133, cx, cy);
			platform->set_color(Yellow);
			csg.drawSolidArc(ang0, ang, 128, 132, cx, cy);
		}
		else
		{
			if (monitoring == RSM) drawGauge(0, Vperm, Yellow);
			drawGauge(0, Vrelease, Black, 132);
			drawGauge(0, Vrelease, MediumGrey, 133);
		}
	}
}
static int initSpeed = 0;
void displayLines()
{
	bool inited = initSpeed == maxSpeed;
	initSpeed = maxSpeed;
	std::unique_ptr<UiPlatform::Font> gaugeFont;
	if (!inited) {
		csg.clear();
		gaugeFont = platform->load_font(16, false);
	}
	platform->set_color(White);

#if SIMRAIL
	int step = 5;
#else
	int step = maxSpeed == 150 ? 5 : 10;
#endif
	for (int i = 0; i <= maxSpeed; i += step)
	{
		float size;
		float an = speedToAngle(i);
#if SIMRAIL
		int longinterval = maxSpeed == 400 ? 50 : (i > maxSpeed / 2 ? 40 : 20);
#else
		int longinterval = maxSpeed == 400 ? 50 : (maxSpeed == 150 ? 25 : 20);
#endif
		size = i % longinterval != 0 ? -110 : -100;
		if (!inited && i % longinterval == 0 && (maxSpeed != 400 || (i != 250 && i != 350)))
		{
			std::string s = to_string(i);
			float hx = 0;
			float hy = 12 / 2.0;

			std::pair<float, float> wh = gaugeFont->calc_size(s);
			hx = wh.first / 2 + 1;
			hy = gaugeFont->ascent() / 2 - 2;
			float maxan = atanf(hy / hx);
			float cuadran = abs(-an - PI / 2);
			float adjust = (abs(PI / 2 - cuadran) > maxan) ? hy / abs(cosf(cuadran)) : hx / sinf(cuadran);
#if SIMRAIL
			float val = size + adjust + 10;
#else
			float val = size + adjust;
#endif
			texture* t = new texture();
			t->x = cx - val * cosf(an);
			t->y = cy - val * sinf(an);
			t->width = wh.first;
			t->height = wh.second;
			t->tex = platform->make_text_image(s, *gaugeFont, White);
			csg.add(t);
		}
		csg.drawRadius(cx, cy, size, -125, an);
}
}
Component releaseRegion(36, 36, displayVrelease);
static float prevVrelease = 0;
bool releaseSignShown = false;
void displayVrelease()
{
	if (mode == Mode::SN && active_ntc_window != nullptr && active_ntc_window->monitoring_data.Vrelease_display & 1) {
		if (prevVrelease != Vrelease || !releaseSignShown)
		{
			releaseSignShown = true;
			releaseRegion.clear();
			releaseRegion.addText(to_string((int)std::round(Vrelease)).c_str(), 0, 0, 17, active_ntc_window->monitoring_data.Vrelease_color, CENTER, 0);
			prevVrelease = Vrelease;
		}
	}
	else if (Vrelease != 0 && Vtarget == 0 && (monitoring == TSM || monitoring == RSM) && (mode != Mode::OS || showSpeeds) && mode != Mode::SN) {
		if (prevVrelease != Vrelease || !releaseSignShown)
		{
			releaseSignShown = true;
			releaseRegion.clear();
			releaseRegion.addText(to_string((int)std::round(Vrelease)).c_str(), 0, 0, 17, MediumGrey, CENTER, 0);
			prevVrelease = Vrelease;
		}
	}
	else {
		releaseSignShown = false;
		releaseRegion.clear();
	}
}
void displayGauge()
{
	if (prevUseImperialSystem != useImperialSystem) {
		maxSpeed = useImperialSystem ? (((int)(etcsDialMaxSpeed * MPH / 20) * 20) + 20) : etcsDialMaxSpeed;
		prevUseImperialSystem = useImperialSystem;
		initSpeed = 0;
		csg.clear();
	}

	displayLines();
	displayCSG();
	drawNeedle();
	drawSetSpeed();
	drawImperialIndicator();
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
	if (mode == Mode::LS) return;
	if ((mode == Mode::FS || ((mode == Mode::OS || mode == Mode::SR) && showSpeeds)) && monitoring == CSM && TTI < TdispTTI)
	{
		if (!ttiShown) playSinfo();
		ttiShown = true;
		int n;
		for (n = 1; n < 11; n++)
		{
			if (TdispTTI * (10 - n) / 10 <= TTI && TTI < TdispTTI * (10 - (n - 1)) / 10) break;
		}
		a1.drawRectangle(0, 0, 50, 50, White, CENTER);
		a1.drawRectangle(0, 0, 5 * n, 5 * n, Grey, CENTER);
	}
	else ttiShown = false;
	if (mode == Mode::FS && (monitoring == TSM || monitoring == RSM) && TTP < TdispTTI) {
		int n;
		for (n = 1; n < 11; n++)
		{
			if (TdispTTI * (10 - n) / 10 <= TTP && TTP < TdispTTI * (10 - (n - 1)) / 10) break;
		}
		a1.drawRectangle(0, 0, 5 * n, 5 * n, supervision == IntS ? Red : ((supervision == WaS || supervision == OvS) ? Orange : Yellow), CENTER);
	}
}

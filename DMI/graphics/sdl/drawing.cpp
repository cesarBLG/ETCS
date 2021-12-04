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
#include <ctime>
#include <chrono>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <deque>
#include <SDL2/SDL.h>
#include "../drawing.h"
#include "../display.h"
#include "../button.h"
#include "../flash.h"
#include "../../sound/sound.h"
#include "../../messages/messages.h"
#include <mutex>
using namespace std;
extern mutex draw_mtx;
SDL_Window *sdlwin;
SDL_Renderer *sdlren;
std::string fontPath = "fonts/swiss.ttf";
std::string fontPathb = "fonts/swissb.ttf";
#define PI 3.14159265358979323846264338327950288419716939937510
float scale = 1;
float offset[2] = {0, 0};
extern bool running;
void quit();
mutex ev_mtx;
void init_video()
{
    int res = SDL_Init(SDL_INIT_EVERYTHING);
    if(res<0)
    {
        printf("Failed to init SDL. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    startDisplay(false);
    int timer = SDL_AddTimer(250, flash, nullptr);
    //SDL_AddTimer(100, [](Uint32 interval, void *) {repaint(); return interval;}, nullptr);
    if(timer == 0)
    {
        printf("Failed to create flashing timer. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    start_sound();
    SDL_Event ev;
    int count = 0;
    chrono::system_clock::time_point lastrender = chrono::system_clock::now() - chrono::hours(1);
    SDL_Event lastPressEvent;
    bool pressEventReceived = false;
    while(running)
    {
        if(SDL_WaitEvent(&ev) != 0)
        {
            /*if(ev.type == SDL_KEYDOWN)
            {
                SDL_KeyboardEvent kev = ev.key;
                if(kev.keysym.sym == SDLK_c)
                {
                    printf("quit\n");
                    running = false;
                }
            }*/
			if(ev.type == SDL_QUIT || ev.type == SDL_WINDOWEVENT_CLOSE)
            {
                quit();
                break;
            }
            if(ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_FINGERDOWN || ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_FINGERUP || ev.type == SDL_MOUSEMOTION || ev.type == SDL_FINGERMOTION)
            {
                pressEventReceived = true;
                lastPressEvent = ev;
            }
			if (pressEventReceived)
			{
                auto prevev = ev;
				ev = lastPressEvent;
				float scrx;
				float scry;
				bool pressed;
				if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERUP || ev.type == SDL_FINGERMOTION)
				{
					SDL_TouchFingerEvent tfe = ev.tfinger;
					pressed = tfe.pressure > 0.1;
					scrx = tfe.x;
					scry = tfe.y;
				}
				else if (ev.type == SDL_MOUSEMOTION)
				{
					SDL_MouseMotionEvent mme = ev.motion;
					pressed = mme.state == SDL_PRESSED;
					scrx = mme.x;
					scry = mme.y;
				}
				else
				{
					SDL_MouseButtonEvent mbe = ev.button;
					pressed = mbe.state == SDL_PRESSED;
					scrx = mbe.x;
					scry = mbe.y;
				}
				extern float scale;
				extern float offset[2];
				float x = (scrx - offset[0]) / scale;
				float y = scry / scale;
				vector<window *> windows;
				unique_lock<mutex> lck(draw_mtx);
				for (auto it = active_windows.begin(); it != active_windows.end(); ++it)
				{
					windows.push_back(*it);
				}
				for (int i = 0; i < windows.size(); i++)
				{
					if (windows[i]->active) windows[i]->event(pressed, x, y);
                    else windows[i]->event(0, -100, -100);
				}
                ev = prevev;
			}
            if(ev.type == SDL_WINDOWEVENT || ev.type == SDL_USEREVENT) 
            {
                auto now = chrono::system_clock::now();
                chrono::duration<double> diff = now - lastrender;
                if(chrono::duration_cast<chrono::duration<int, milli>>(diff).count() > 50)
                {
                    lastrender = chrono::system_clock::now();
                    display();
                }
            }
        }
    }
    quitDisplay();
}
void startDisplay(bool fullscreen)
{
    TTF_Init();
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    sdlwin = SDL_CreateWindow("Driver Machine Interface", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if(sdlwin == nullptr)
    {
        printf("Failed to create window. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    if(fullscreen) SDL_SetWindowFullscreen(sdlwin, SDL_WINDOW_FULLSCREEN_DESKTOP); 
    sdlren = SDL_CreateRenderer(sdlwin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(sdlren == nullptr)
    {
        printf("Failed to create renderer. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    int w,h;
    SDL_GetWindowSize(sdlwin, &w, &h);
    float scrsize[] = {(float)w,(float)h};
    float extra = 640/2*(scrsize[0]/(scrsize[1]*4/3)-1);
    offset[0] = extra;
    scale = scrsize[1]/480.0;
    //SDL_SetWindowBordered(sdlwin, SDL_FALSE);
}
void display()
{
    auto start = chrono::system_clock::now();
    clear();
    unique_lock<mutex> lck(draw_mtx);
    displayETCS();
    SDL_RenderPresent(sdlren);
    lck.unlock();
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end-start;
    //printf("%f\n", diff.count());
}
void quitDisplay()
{
    SDL_DestroyRenderer(sdlren);
    SDL_DestroyWindow(sdlwin);
    SDL_Quit();
    exit(nullptr);
}
void clear()
{
    setColor(DarkBlue);
    SDL_RenderClear(sdlren);
}
Color renderColor;
void setColor(Color color)
{
    renderColor = color;
    int res = SDL_SetRenderDrawColor(sdlren, color.R,color.G,color.B,255);
    if(res<0) printf("Failed to set render color. SDL Error: %s\n", SDL_GetError());
}
inline int getScale(float val)
{
    return round(val*scale);
}
float getAntiScale(float val)
{
    return val/scale;
}
void getFontSize(TTF_Font *font, const char *str, float *width, float *height)
{
    int w;
    int h;
    TTF_SizeText(font, str, &w, &h);
    *width = w/scale;
    *height = h/scale;
}
TTF_Font *openFont(std::string text, float size)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    text = filesDir+"/"+text;
#endif
    TTF_Font *f = TTF_OpenFont(text.c_str(), getScale(size)*1.4);
    if(f == nullptr)
    {
        printf("Error loading font %s. SDL Error: %s\n", text.c_str(), SDL_GetError());
        return nullptr;
    }
    /*float height = TTF_FontAscent(f)/scale;
    TTF_CloseFont(f);
    f = TTF_OpenFont(text, getScale(size)*size/height);*/
    return f;
}
void repaint()
{
    SDL_Event ev;
    //unique_lock<mutex> l(ev_mtx);
    ev.type = SDL_USEREVENT;
    int result = SDL_PushEvent(&ev);
    if(result < 0) printf("SDL Event error: %s\n", SDL_GetError());
}

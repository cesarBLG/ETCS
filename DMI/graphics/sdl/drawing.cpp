/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <ctime>
#include <chrono>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <deque>
#include <SDL.h>
#include <thread>
#include <mutex>
#include "../drawing.h"
#include "../display.h"
#include "../button.h"
#include "../flash.h"
#include "../../sound/sound.h"
#include "../../messages/messages.h"
#include "../../tcp/server.h"
#include "../../Settings/settings.h"
using namespace std;
extern mutex draw_mtx;
SDL_Window* sdlwin;
SDL_Renderer* sdlren;
SDL_Texture* sdltex;
bool renderToTexture;
bool sdlrot;
#if SIMRAIL
std::string fontPath = "fonts/verdana.ttf";
std::string fontPathb = "fonts/verdanab.ttf";
#else
std::string fontPath = "fonts/swiss.ttf";
std::string fontPathb = "fonts/swissb.ttf";
#endif
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

    startDisplay(
        Settings::Get("fullScreen") == "true",
        atoi(Settings::Get("display").c_str()),
        atoi(Settings::Get("width").c_str()),
        atoi(Settings::Get("height").c_str()),
        Settings::Get("borderless") == "true",
        Settings::Get("rotateScreen") == "true"
    );

    int timer = SDL_AddTimer(250, flash, nullptr);
    if(timer == 0)
    {
        printf("Failed to create flashing timer. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    start_sound();
}
void loop_video()
{
    while(running)
    {
        auto prev = std::chrono::system_clock::now();
        SDL_Event ev;
        while(SDL_PollEvent(&ev) != 0)
        {
			if(ev.type == SDL_QUIT || ev.type == SDL_WINDOWEVENT_CLOSE)
            {
                quit();
                break;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_MOUSEMOTION /*|| ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERUP || ev.type == SDL_FINGERMOTION*/) {
				float scrx;
				float scry;
				bool pressed;
                if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERUP || ev.type == SDL_FINGERMOTION)
                {
                    SDL_TouchFingerEvent tfe = ev.tfinger;
                    if (ev.type == SDL_FINGERMOTION) pressed = tfe.pressure > 0;
                    else pressed = ev.type == SDL_FINGERDOWN;
                    scrx = sdlrot ? 640 - tfe.x : tfe.x;
                    scry = sdlrot ? 480 - tfe.y : tfe.y;
                }
                else if (ev.type == SDL_MOUSEMOTION)
                {
                    SDL_MouseMotionEvent mme = ev.motion;
                    pressed = mme.state == SDL_PRESSED;
                    scrx = sdlrot ? 640 - mme.x : mme.x;
                    scry = sdlrot ? 480 - mme.y : mme.y;
                }
                else
                {
                    SDL_MouseButtonEvent mbe = ev.button;
                    pressed = mbe.state == SDL_PRESSED;
                    scrx = sdlrot ? 640 - mbe.x : mbe.x;
                    scry = sdlrot ? 480 - mbe.y : mbe.y;
                }
				extern float scale;
				extern float offset[2];
				float x = (scrx - offset[0]) / scale;
				float y = scry / scale;
				vector<window *> windows;
				for (auto it = active_windows.begin(); it != active_windows.end(); ++it)
				{
					windows.push_back(*it);
				}
				for (int i = 0; i < windows.size(); i++)
				{
					if (windows[i]->active) windows[i]->event(pressed, x, y);
                    else windows[i]->event(0, -100, -100);
				}
			}
        }
        if (!running) break;
        void update_stm_windows();
        updateDrawCommands();
        update_stm_windows();
        display();

        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        int d = std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(diff).count();
        int time = 50 - d;
        if (time < 30)
        {
            time = 125 - d;
            if (time < 60)
            {
                time = 250 - d;
                if (time < 100) time = 2*d;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
    quitDisplay();
}
void startDisplay(bool fullscreen, int display = 0, int width = 640, int height = 480, bool borderless = false, bool rotate = false)
{
    TTF_Init();
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D_THREADSAFE, "1");
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    sdlwin = SDL_CreateWindow("Driver Machine Interface", SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display), width, height, SDL_WINDOW_SHOWN);
    if(sdlwin == nullptr)
    {
        printf("Failed to create window. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    if (fullscreen)
        SDL_SetWindowFullscreen(sdlwin, SDL_WINDOW_FULLSCREEN_DESKTOP);

    sdlren = SDL_CreateRenderer(sdlwin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (sdlren == nullptr)
    {
        printf("Failed to create renderer. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    sdlrot = rotate;

    if (width < 640 || height < 480) renderToTexture = true;

    if (renderToTexture)
    {
        sdltex = SDL_CreateTexture(sdlren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_RenderSetLogicalSize(sdlren, 640, 480);
    }
    else
    {
        int w = 640, h = 480;
        SDL_GetWindowSize(sdlwin, &w, &h);
        float scrsize[] = { (float)w,(float)h };
        float extra = 640 / 2 * (scrsize[0] / (scrsize[1] * 4 / 3) - 1);
        offset[0] = extra;
        scale = scrsize[1] / 480.0;
    }

    if (borderless)
        SDL_SetWindowBordered(sdlwin, SDL_FALSE);

}
void display()
{
    if (renderToTexture) SDL_SetRenderTarget(sdlren, sdltex);
    clear();
    displayETCS();
    if (renderToTexture)
    {
        SDL_SetRenderTarget(sdlren, nullptr);
        SDL_RenderCopyEx(sdlren, sdltex, nullptr, nullptr, sdlrot ? 180.0f : 0.0f, nullptr, SDL_FLIP_NONE);
    }
    SDL_RenderPresent(sdlren);
}

void quitDisplay()
{
    SDL_DestroyRenderer(sdlren);
    SDL_DestroyWindow(sdlwin);
    SDL_Quit();
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
int getScale(float val)
{
    return round(val*scale);
}
float getAntiScale(float val)
{
    return val/scale;
}
void getFontSize(TTF_Font *font, const char *str, float *width, float *height)
{
    int w=0;
    int h=0;
    TTF_SizeUTF8(font, str, &w, &h);
    *width = w/scale;
    *height = h/scale;
}
std::map<std::pair<std::string, float>, TTF_Font*> fonts;
TTF_Font *openFont(std::string text, float size)
{
    auto it = fonts.find({text,size});
    if (it == fonts.end())
    {
        std::string path = text;
#ifdef __ANDROID__
        extern std::string filesDir;
        path = filesDir+"/"+text;
#endif

#if SIMRAIL
        TTF_Font* f = TTF_OpenFont(path.c_str(), getScale(size) * 1.25);
#else
        TTF_Font* f = TTF_OpenFont(path.c_str(), getScale(size) * 1.4);
#endif
        if(f == nullptr) printf("Error loading font %s. SDL Error: %s\n", path.c_str(), SDL_GetError());
        it = fonts.insert({{text,size}, f}).first;
    }
    return it->second;
}
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
#include <fstream>
#include "../drawing.h"
#include "../display.h"
#include "../button.h"
#include "../flash.h"
#include "../../sound/sound.h"
#include "../../messages/messages.h"
#include "../../tcp/server.h"
#include "../../platform/sdl_platform.h"
//#include "../../platform/null_platform.h"
using namespace std;

static SDL_Window *sdlwin;
static SDL_Renderer *sdlren;
float scale, ox, oy;

static std::map<std::string, std::string>  ini_items;

static void ini_load()
{
    std::ifstream file("settings.ini", std::ios::binary);
    std::string line;

    while (std::getline(file, line)) {
        while (!line.empty() && line.back() == '\r' || line.back() == '\n')
            line.pop_back();
        int pos = line.find('=');
        if (pos == -1)
            continue;
        ini_items.insert(std::pair<std::string, std::string>(line.substr(0, pos), line.substr(pos+1)));
    }
}

static std::string ini_get(const std::string &key)
{
    auto it = ini_items.find(key);
    if (it == ini_items.end())
        return "";
    return it->second;
}

void update_stm_windows();

void present_frame()
{
    updateDrawCommands();
    update_stm_windows();
    platform->set_color(DarkBlue);
    platform->clear();
    displayETCS();
    platform->present().then(present_frame).detach();
}

void process_input(UiPlatform::InputEvent ev)
{
    vector<window *> windows;
    for (auto it = active_windows.begin(); it != active_windows.end(); ++it)
    {
        windows.push_back(*it);
    }
    for (int i = 0; i < windows.size(); i++)
    {
        if (windows[i]->active) windows[i]->event(ev.action != UiPlatform::InputEvent::Action::Release, ev.x, ev.y);
        else windows[i]->event(0, -100, -100);
    }
    platform->on_input_event().then(process_input).detach();
}

void drawing_start()
{
    platform->on_quit_request().then([](){
        platform->quit();
    }).detach();

    platform->on_quit().then([](){
        platform = nullptr;
        SDL_DestroyRenderer(sdlren);
        SDL_DestroyWindow(sdlwin);
        SDL_Quit();
    }).detach();

    platform->on_input_event().then(process_input).detach();

    setupFlash();
    present_frame();
}

void startDisplay(bool fullscreen, int display = 0, int width = 640, int height = 480, bool borderless = false, bool rotate = false)
{
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D_THREADSAFE, "1");
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    sdlwin = SDL_CreateWindow("Driver Machine Interface", SDL_WINDOWPOS_CENTERED_DISPLAY(display), SDL_WINDOWPOS_CENTERED_DISPLAY(display), width, height, SDL_WINDOW_SHOWN);
    if(sdlwin == nullptr)
    {
        printf("Failed to create window. SDL Error: %s", SDL_GetError());
        return;
    }
    if (fullscreen)
        SDL_SetWindowFullscreen(sdlwin, SDL_WINDOW_FULLSCREEN_DESKTOP);

    sdlren = SDL_CreateRenderer(sdlwin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (sdlren == nullptr)
    {
        printf("Failed to create renderer. SDL Error: %s", SDL_GetError());
        return;
    }

    int wx, wy;
    SDL_GetWindowSize(sdlwin, &wx, &wy);
    float sx = wx / 640.0f;
    float sy = wy / 480.0f;
    scale = std::min(sx, sy);
    if (sx > sy) {
        ox = (wx - wy * (640.0f / 480.0f)) * 0.5f;
        oy = 0.0f;
    } else {
        ox = 0.0f;
        oy = (wy - wx * (480.0f / 640.0f)) * 0.5f;
    }
    if (rotate) {
        scale *= -1.0f;
        ox += wx - ox * 2.0f;
        oy += wy - oy * 2.0f;
    }

    if (borderless)
        SDL_SetWindowBordered(sdlwin, SDL_FALSE);

    platform = std::make_unique<SdlPlatform>(sdlren, scale, ox, oy);
    //platform = std::make_unique<NullPlatform>();
}

void init_video()
{
    int res = SDL_Init(SDL_INIT_EVERYTHING);
    if(res<0)
    {
        printf("Failed to init SDL. SDL Error: %s", SDL_GetError());
        return;
    }

    ini_load();
    startDisplay(
        ini_get("fullScreen") == "true",
        std::stoi(ini_get("display")),
        std::stoi(ini_get("width")),
        std::stoi(ini_get("height")),
        ini_get("borderless") == "true",
        ini_get("rotateScreen") == "true"
    );

    loadBeeps();
}

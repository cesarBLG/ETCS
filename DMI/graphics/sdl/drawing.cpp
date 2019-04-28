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
char *fontPath = "fonts/swiss.ttf";
char *fontPathb = "fonts/swissb.ttf";
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
    if(timer == 0)
    {
        printf("Failed to create flashing timer. SDL Error: %s", SDL_GetError());
        running = false;
        return;
    }
    SDL_Event ev;
    int count = 0;
    chrono::system_clock::time_point lastrender = chrono::system_clock::now() - chrono::hours(1);
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
            }
            else if(ev.type == SDL_WINDOWEVENT || ev.type == SDL_USEREVENT) 
            {
                auto now = chrono::system_clock::now();
                chrono::duration<double> diff = now - lastrender;
                checkSound();
                if(chrono::duration_cast<chrono::duration<int, milli>>(diff).count() > 50)
                {
                    lastrender = chrono::system_clock::now();
                    display();
                }
            }
            else if(ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_FINGERDOWN)
            {
                float scrx;
                float scry;
                if(ev.type == SDL_FINGERDOWN)
                {
                    SDL_TouchFingerEvent tfe = ev.tfinger;
                    scrx = tfe.x;
                    scry = tfe.y;
                }
                else
                {
                    SDL_MouseButtonEvent mbe = ev.button;
                    scrx = mbe.x;
                    scry = mbe.y;
                }
                extern float scale;
                extern float offset[2];
                float x = (scrx-offset[0]) / scale;
                float y = scry / scale;
                vector<window*> windows;
                draw_mtx.lock();
                for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
                {
                    if((*it)->active) windows.push_back(*it);
                }
                for(int i=0; i<windows.size(); i++)
                {
                    windows[i]->event(1, x, y);
                }
                draw_mtx.unlock();
            }
        }
    }
    quitDisplay();
}
void startDisplay(bool fullscreen)
{
    TTF_Init();
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
    float scrsize[] = {w,h};
    float extra = 640/2*(scrsize[0]/(scrsize[1]*4/3)-1);
    offset[0] = extra;
    scale = scrsize[1]/480.0;
}
void display()
{
    auto start = chrono::system_clock::now();
    clear();
    draw_mtx.lock();
    displayETCS();
    draw_mtx.unlock();
    SDL_RenderPresent(sdlren);
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end-start;
    printf("%f\n", diff.count());
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
TTF_Font *openFont(char *text, float size)
{
    TTF_Font *f = TTF_OpenFont(text, getScale(size)*1.4);
    if(f == nullptr)
    {
        printf("Error loading font %s. SDL Error: \n", text, SDL_GetError());
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
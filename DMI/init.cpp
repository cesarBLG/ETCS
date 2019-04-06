#include "monitor.h"
#include "graphics/drawing.h"
#include "graphics/display.h"
#include "graphics/window.h"
#include "graphics/button.h"
#include "tcp/server.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <vector>
#include "sound/sound.h"
#include "graphics/flash.h"
#include <deque>
#include "messages/messages.h"
#include <algorithm>
using namespace std;
bool running = true;
#ifdef __unix__
#include <signal.h>
void sighandler(int sig)
{
    running = false;
}
#endif
int main(int argc, char** argv)
{
#ifdef __unix__
    signal(SIGINT, sighandler);
#endif
    mode = SB;
    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    startDisplay(false);
    thread tcp(startSocket);
    int timer = SDL_AddTimer(250, flash, nullptr);
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
                printf("quit\n");
                running = false;
            }
            else if(ev.type == SDL_WINDOWEVENT || ev.type == SDL_USEREVENT) 
            {
                auto now = chrono::system_clock::now();
                chrono::duration<double> diff = now - lastrender;
                checkSound();
                if(chrono::duration_cast<chrono::duration<int, milli>>(diff).count() > 80)
                {
                    lastrender = chrono::system_clock::now();
                    update();
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
                for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
                {
                    windows.push_back(*it);
                }
                for(int i=0; i<windows.size(); i++)
                {
                    windows[i]->event(1, x, y);
                }
            }
        }
    }
    tcp.join();
    quitDisplay();
    return 0;
}

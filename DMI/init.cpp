#include "monitor.h"
#include "graphics/drawing.h"
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
            else if(ev.type == SDL_MOUSEBUTTONDOWN)
            {
                SDL_MouseButtonEvent mbe = ev.button;
                /*extern deque<Message> messageList;
                bool c = false;
                for(int i = 0; i<messageList.size(); i++)
                {
                    if(messageList[i].ack)
                    {
                        messageList[i].ack = false;
                        c = true;
                        playClick();
                        break;
                    }
                }
                if(c) continue;*/
                extern float scale;
                float x = mbe.x * scale;
                float y = mbe.y * scale;
                extern vector<window*> active_windows;
                for(int i=0; i<active_windows.size(); i++)
                {
                    active_windows[i]->event(1, x, y);
                }
            }
        }
    }
    tcp.join();
    quitDisplay();
    return 0;
}

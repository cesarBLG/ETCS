#include "flash.h"
#include <cstdio>
using namespace std;
static SDL_Event ev;
int flash_state = 0;
Uint32 flash(Uint32 interval, void *param)
{
    flash_state++;
    if(flash_state == 4) flash_state = 0;
    ev.type = SDL_USEREVENT;
    int result = SDL_PushEvent(&ev);
    if(result < 0) printf("SDL Event error: %s\n", SDL_GetError());
    return interval;
}
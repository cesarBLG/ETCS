#ifndef _FLASH_H
#define _FLASH_H
#include <SDL2/SDL.h>
extern int flash_state;
Uint32 flash(Uint32 interval, void *param);
#endif
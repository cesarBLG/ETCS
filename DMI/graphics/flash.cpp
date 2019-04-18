#include "flash.h"
#include <cstdio>
#include "drawing.h"
using namespace std;
int flash_state = 0;
Uint32 flash(Uint32 interval, void *param)
{
    flash_state++;
    if(flash_state == 4) flash_state = 0;
    repaint();
    return interval;
}
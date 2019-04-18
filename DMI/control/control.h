#ifndef _CONTROL_H
#define _CONTROL_H
#include "../window/window.h"
class window;
extern bool running;
void prepareLayout();
void right_menu(window *w);
void exit(window *w);
#endif
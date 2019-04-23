#ifndef _CONTROL_H
#define _CONTROL_H
#include "../window/window.h"
class window;
class subwindow;
extern bool running;
void prepareLayout();
void right_menu(subwindow *w);
void exit(subwindow *w);
#endif
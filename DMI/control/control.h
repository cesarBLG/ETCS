#ifndef _CONTROL_H
#define _CONTROL_H
#include "../window/window.h"
#include <mutex>
#include <condition_variable> 
class window;
class subwindow;
extern mutex window_mtx;
extern condition_variable window_cv;
void manage_windows();
void right_menu(subwindow *w);
void exit(subwindow *w);
#endif
#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <unordered_set>
#include "window.h"
#include "component.h"
using namespace std;
extern window main_window;
extern window navigation_bar;
//extern window menu_main;
extern window PASP;
extern unordered_set<window*> active_windows;
extern unordered_set<window*> invalid_windows;
void displayETCS();
void prepareLayout();
#endif
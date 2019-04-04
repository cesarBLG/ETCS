#include <ctime>
#include <chrono>
#include "display.h"
#include <cstdio>
#include "button.h"
#include "text_button.h"
#include "menu.h"
using namespace std;
unordered_set<window*> active_windows;
unordered_set<window*> invalid_windows;
#include <iostream>
void displayETCS()
{
    auto start = chrono::system_clock::now();
    for(auto it=invalid_windows.begin(); it!=active_windows.end(); ++it)
    {
        window *w = *it;
        active_windows.erase(w);
        delete w;
    }
    invalid_windows.clear();
    for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
    {
        (*it)->display();
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end-start;
    //printf("%f\n", diff.count());
}
void prepareLayout()
{
    active_windows.insert(&navigation_bar);
    active_windows.insert(&main_window);
    active_windows.insert(&planning_area);
    for(auto it=active_windows.begin(); it!=active_windows.end(); ++it)
    {
        (*it)->construct();
    }
}
void exit(window *w)
{
    active_windows.insert(&navigation_bar);
    active_windows.insert(&planning_area);
    invalid_windows.insert(w);
}

#include "control.h"
#include "../graphics/display.h"
#include "../graphics/drawing.h"
#include "../monitor.h"
#include "../window/level_window.h"
#include "../window/driver_id.h"
#include <thread>
#include <iostream>
using namespace std;
extern bool running;
static bool ex = false;
static subwindow *c = nullptr;
mutex window_mtx;
mutex draw_mtx;
condition_variable window_cv;
bool aw(){return ex||c!=nullptr||!running;}
void wait(subwindow *w)
{
    draw_mtx.lock();
    active_windows.insert(w);
    draw_mtx.unlock();
    int i=0;
    while(!ex)
    {
        navigation_bar.active = false;
        planning_area.active = false;
        if(w->fullscreen) main_window.active = false;
        repaint();
        unique_lock<mutex> lck(window_mtx);
        window_cv.wait(lck, aw);
        if(!running) break;
        if(c!=nullptr)
        {
            bool e = ex;
            subwindow *w1 = c;
            c = nullptr;
            ex = false;
            w->active = false;
            lck.unlock();
            wait(w1);
            w->active = true;
            ex = e;
        }
    }
    ex = false;
    draw_mtx.lock();
    active_windows.erase(w);
    if(w->fullscreen) main_window.active = true;
    navigation_bar.active = true;
    planning_area.active = true;
    draw_mtx.unlock();
    delete w;
}
void manage_windows()
{
    main_window.construct();
    navigation_bar.construct();
    planning_area.construct();
    active_windows.insert(&main_window);
    active_windows.insert(&navigation_bar);
    active_windows.insert(&planning_area);
    while(running && driverid==0)
    {
        wait(new driver_window());
    }
    do
    {
        wait(new level_window());
    }
    while(running && level==Unknown);
    navigation_bar.active = true;
    planning_area.active = true;
    while(running)
    {
        unique_lock<mutex> lck(window_mtx);
        window_cv.wait(lck, aw);
        if(!running) break;
        if(c!=nullptr)
        {
            subwindow *w = c;
            c = nullptr;
            lck.unlock();
            wait(w);
        }
    }
}
void right_menu(subwindow *w)
{
    unique_lock<mutex> lck(window_mtx);
    c = w;
    window_cv.notify_one();
}
void exit(subwindow *w)
{
    unique_lock<mutex> lck(window_mtx);
    ex = true;
    window_cv.notify_one();
}

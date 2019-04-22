#include "control.h"
#include "../graphics/display.h"
#include "../graphics/drawing.h"
#include "../monitor.h"
#include "../window/level_window.h"
#include "../window/driver_id.h"
#include <thread>
#include <mutex>
#include <condition_variable> 
#include <iostream>
using namespace std;
static bool ex = false;
static window *c = nullptr;
mutex window_mtx;
mutex draw_mtx;
condition_variable cv;
bool aw(){return ex||c!=nullptr||!running;}
void wait(window *w)
{
    draw_mtx.lock();
    active_windows.insert(w);
    draw_mtx.unlock();
    repaint();
    int i=0;
    while(!ex)
    {
        navigation_bar.active = false;
        planning_area.active = false;
        main_window.active = false;
        unique_lock<mutex> lck(window_mtx);
        cv.wait(lck, aw);
        if(!running) break;
        if(c!=nullptr)
        {
            bool e = ex;
            window *w1 = c;
            c = nullptr;
            ex = false;
            w->active = false;
            lck.unlock();
            wait(w1);
            w->active = true;
            ex = e | true;
        }
    }
    ex = false;
    draw_mtx.lock();
    active_windows.erase(w);
    delete w;
    draw_mtx.unlock();
    navigation_bar.active = true;
    planning_area.active = true;
    main_window.active = true;
}
void prepareLayout()
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
        cv.wait(lck, aw);
        if(!running) break;
        if(c!=nullptr)
        {
            window *w = c;
            c = nullptr;
            lck.unlock();
            wait(w);
        }
    }
}
void right_menu(window *w)
{
    unique_lock<mutex> lck(window_mtx);
    c = w;
    cv.notify_one();
}
void exit(window *w)
{
    unique_lock<mutex> lck(window_mtx);
    ex = true;
    cv.notify_one();
}

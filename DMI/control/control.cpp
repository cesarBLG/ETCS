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
static bool exit_menu = false;
static subwindow *c = nullptr;
mutex window_mtx;
mutex draw_mtx;
condition_variable window_cv;
bool wake_fun(){return exit_menu||c!=nullptr||!running;}
void wait(subwindow *w)
{
    draw_mtx.lock();
    active_windows.insert(w);
    draw_mtx.unlock();
    while(!exit_menu)
    {
        draw_mtx.lock();
        navigation_bar.active = false;
        planning_area.active = false;
        if(w->fullscreen) main_window.active = false;
        draw_mtx.unlock();
        repaint();
        unique_lock<mutex> lck(window_mtx);
        window_cv.wait(lck, wake_fun);
        if(!running) break;
        if(c!=nullptr)
        {
            bool e = exit_menu;
            subwindow *w1 = c;
            c = nullptr;
            exit_menu = false;
            draw_mtx.lock();
            w->active = false;
            draw_mtx.unlock();
            lck.unlock();
            wait(w1);
            window_mtx.lock();
            draw_mtx.lock();
            w->active = true;
            exit_menu = e;
            draw_mtx.unlock();
            window_mtx.unlock();
        }
    }
    window_mtx.lock();
    exit_menu = false;
    window_mtx.unlock();
    draw_mtx.lock();
    active_windows.erase(w);
    old_windows.insert(w);
    if(w->fullscreen) main_window.active = true;
    navigation_bar.active = true;
    planning_area.active = true;
    draw_mtx.unlock();
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
        std::this_thread::sleep_for(100ms);
    }
    while(running && level==Unknown)
    {
        wait(new level_window());
        std::this_thread::sleep_for(100ms);
    }
    navigation_bar.active = true;
    planning_area.active = true;
    while(running)
    {
        unique_lock<mutex> lck(window_mtx);
        window_cv.wait(lck, wake_fun);
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
    lck.unlock();
    window_cv.notify_one();
}
void exit(subwindow *w)
{
    unique_lock<mutex> lck(window_mtx);
    exit_menu = true;
    lck.unlock();
    window_cv.notify_one();
}

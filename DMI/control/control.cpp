/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
static function<void()> proc = nullptr;
mutex window_mtx;
mutex draw_mtx;
condition_variable window_cv;
bool wake_fun(){return exit_menu||c!=nullptr||proc!=nullptr||!running;}
void set_procedure(function<void()> p)
{
    std::unique_lock<mutex> lck(window_mtx);
    ::proc = p;
    lck.unlock();
    window_cv.notify_one();
}
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
        if(!running || proc != nullptr)
        {
            lck.unlock();
            break;
        }
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
void start_procedure()
{
    navigation_bar.active = false;
    planning_area.active = false;
    while(running && (driverid=="" || !driverid_valid))
    {
        wait(new driver_window());
        if (proc != nullptr) return;
        std::this_thread::sleep_for(100ms);
    }
    while(running && (level==Level::Unknown || !level_valid))
    {
        if (level != Level::Unknown) 
        {
            wait(new level_validation_window());
        if (proc != nullptr) return;
            std::this_thread::sleep_for(100ms);
        }
        if (level_valid) break;
        wait(new level_window());
        if (proc != nullptr) return;
        std::this_thread::sleep_for(100ms);
    }
    navigation_bar.active = true;
    planning_area.active = true;
}
void manage_windows()
{
    main_window.construct();
    navigation_bar.construct();
    planning_area.construct();
    active_windows.insert(&main_window);
    active_windows.insert(&navigation_bar);
    active_windows.insert(&planning_area);
    navigation_bar.active = true;
    planning_area.active = true;
    while(running)
    {
        unique_lock<mutex> lck(window_mtx);
        window_cv.wait(lck, wake_fun);
        if(!running) break;
        if (proc!=nullptr)
        {
            auto p = proc;
            proc = nullptr;
            lck.unlock();
            p();
        }
        else if(c!=nullptr)
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

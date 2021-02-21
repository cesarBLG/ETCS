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
#include "monitor.h"
#include "graphics/drawing.h"
#include "control/control.h"
#include "tcp/server.h"
#include <thread>
using namespace std;
bool running = true;
void quit()
{
    unique_lock<mutex> lck(window_mtx);
    running = false;
    window_cv.notify_one();
    printf("quit\n");
}
#ifdef __ANDROID__
#else
#ifdef __unix__
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cxxabi.h>
#include <time.h>
void sighandler(int sig)
{
    quit();
}
void crash_handler(int sig)
{
    void *bt[20];
    size_t size = backtrace(bt, 20);
    time_t now = time(nullptr);
    fprintf(stderr, "DMI received signal %d, date: %s\n", sig, ctime(&now));
    backtrace_symbols_fd(bt, size, 2);
    running = false;
    exit(1);
}
#endif
#endif
int main(int argc, char** argv)
{
#ifdef __ANDROID__
#else
#ifdef __unix__
    signal(SIGINT, sighandler);
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#endif
#endif
    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    thread video(init_video);
    thread tcp(startSocket);
    manage_windows();
    tcp.join();
    video.join();
    return 0;
}

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
int main(int argc, char** argv)
{
#ifdef __unix__
    signal(SIGINT, sighandler);
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
#endif
    mode = SB;
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

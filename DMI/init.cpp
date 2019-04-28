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
#include <signal.h>
void sighandler(int sig)
{
    quit();
}
#endif
int main(int argc, char** argv)
{
#ifdef __unix__
    signal(SIGINT, sighandler);
#endif
    mode = SB;
    setSpeeds(0, 0, 0, 0, 0, 0);
    setMonitor(CSM);
    setSupervision(NoS);
    thread tcp(startSocket);
    thread video(init_video);
    manage_windows();
    tcp.join();
    video.join();
    return 0;
}

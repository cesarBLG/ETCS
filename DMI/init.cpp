#include "monitor.h"
#include "graphics/drawing.h"
#include "control/control.h"
#include "tcp/server.h"
#include <thread>
using namespace std;
bool running = true;
#ifdef __unix__
#include <signal.h>
void sighandler(int sig)
{
    running = false;
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
    prepareLayout();
    tcp.join();
    video.join();
    return 0;
}

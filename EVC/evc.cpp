#include <thread>
#include <cstdio>
#include "DMI/dmi.h"
using namespace std;
int main()
{
    printf("Starting European Train Control System...\n");
    thread dmi_thread(start_dmi);
    dmi_thread.join();
    exit(0);
}

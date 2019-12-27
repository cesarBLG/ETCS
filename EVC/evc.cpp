#include <thread>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include "DMI/dmi.h"
#include <unistd.h>
#include <iostream>
#include <chrono>
//#include "packet_reader.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/distance.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
std::mutex loop_mtx;
std::condition_variable loop_notifier;
void start();
void loop();
int main()
{
    std::printf("Starting European Train Control System...\n");
    start();
    loop();
    return 0;
}
void start()
{
    start_dmi();
    //start_or_iface();
    //start_packet_reader();
    mrsp_candidates.insert_restriction(new speed_restriction(120/3.6, distance(std::numeric_limits<double>::min()), distance(std::numeric_limits<double>::max()), false));
    mrsp_candidates.insert_restriction(new speed_restriction(90/3.6, distance(0), distance(1200), true));
    mrsp_candidates.insert_restriction(new speed_restriction(30/3.6, distance(1000), distance(1100), true));
    mrsp_candidates.insert_restriction(new speed_restriction(80/3.6, distance(1100), distance(3000), true));
    mrsp_candidates.insert_restriction(new speed_restriction(60/3.6, distance(3000), distance(4000), true));
    set_test_values();
}
void loop()
{
    V_est = 12;
    while(1)
    {
        std::unique_lock<std::mutex> lck(loop_mtx);
        update_train_position();
        auto prev = std::chrono::system_clock::now();
        update_supervision();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        //std::cout<<std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count()<<std::endl;
        lck.unlock();
        usleep(50000);
        //loop_notifier.wait(lck);
    }
}

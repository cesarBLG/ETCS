#include <thread>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include "DMI/dmi.h"
#include <iostream>
#include <chrono>
//#include "packet_reader.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/distance.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
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
    start_or_iface();
    //start_packet_reader();
    mrsp_candidates.insert_restriction(new speed_restriction(V_train, distance(std::numeric_limits<double>::lowest()), distance(std::numeric_limits<double>::max()), false));
    set_test_values();
}
void loop()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(loop_mtx);
        //update_train_position();
        auto prev = std::chrono::system_clock::now();
        update_supervision();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        //std::cout<<std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count()<<std::endl;
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //loop_notifier.wait(lck);
    }
}

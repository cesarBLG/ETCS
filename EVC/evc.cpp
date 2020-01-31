#include <thread>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include "DMI/dmi.h"
#include <iostream>
#include <chrono>
#include "Packets/messages.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/national_values.h"
#include "Position/distance.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
#include "Procedures/mode_transition.h"
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
    setup_national_values();
    set_test_values();
    initialize_mode_transitions();
    ETCS_packet::initialize();
}
void loop()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(loop_mtx);
        auto prev = std::chrono::system_clock::now();
        update_odometer();
        update_mode_status();
        check_eurobalise_passed();
        if (mode == Mode::OS || mode == Mode::FS || mode == Mode::LS || mode == Mode::SR) update_supervision();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        //std::cout<<std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count()<<std::endl;
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

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
#include "NationalFN/nationalfn.h"
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
    initialize_national_functions();
}
void loop()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(loop_mtx);
        auto prev = std::chrono::system_clock::now();
        update_odometer();
        check_eurobalise_passed();
        update_mode_status();
        if (mode == Mode::OS || mode == Mode::FS || mode == Mode::LS || mode == Mode::SN ||
        mode == Mode::SR || mode == Mode::SH || mode == Mode::UN || mode == Mode::RV)
             update_supervision();
        update_national_functions();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        //std::cout<<std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count()<<std::endl;
        lck.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

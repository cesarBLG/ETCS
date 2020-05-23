#include <thread>
#include <cstdio>
#include <mutex>
#include <condition_variable>
#include "DMI/dmi.h"
#include "DMI/text_message.h"
#include <iostream>
#include <chrono>
#include "Packets/messages.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/national_values.h"
#include "Position/distance.h"
#include "Position/geographical.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
#include "Procedures/procedures.h"
#include "NationalFN/nationalfn.h"
#include "TrackConditions/track_condition.h"
#include "TrainSubsystems/subsystems.h"
#include "evc.h"
std::mutex loop_mtx;
std::condition_variable loop_notifier;
void loop();
int main()
{
    std::printf("Starting European Train Control System...\n");
    start();
    loop();
    return 0;
}
bool started=false;
void start()
{
    start_dmi();
    start_or_iface();
    setup_national_values();
    initialize_mode_transitions();
    ETCS_packet::initialize();
    set_message_filters();
    initialize_national_functions();
    started = true;
}
void update()
{
    std::unique_lock<std::mutex> lck(loop_mtx);
    update_odometer();
    update_geographical_position();
    check_eurobalise_passed();
    update_procedures();
    update_supervision();
    update_track_conditions();
    update_messages();
    update_national_functions();
    update_train_subsystems();
}
void loop()
{
    while(1)
    {
        auto prev = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - prev;
        //std::cout<<std::chrono::duration_cast<std::chrono::duration<int, std::micro>>(diff).count()<<std::endl;
        update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

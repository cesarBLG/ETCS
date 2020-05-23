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

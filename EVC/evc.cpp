/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "TrainSubsystems/cold_movement.h"
#include "DMI/dmi.h"
#include "DMI/text_message.h"
#include "DMI/windows.h"
#include "DMI/track_ahead_free.h"
#include "Packets/messages.h"
#include "Packets/logging.h"
#include "Supervision/speed_profile.h"
#include "Supervision/targets.h"
#include "Supervision/supervision.h"
#include "Supervision/locomotive_data.h"
#include "Position/distance.h"
#include "Position/geographical.h"
#include "Supervision/conversion_model.h"
#include "OR_interface/interface.h"
#include "MA/movement_authority.h"
#include "Procedures/procedures.h"
#include "NationalFN/nationalfn.h"
#include "TrackConditions/track_condition.h"
#include "TrainSubsystems/subsystems.h"
#include "LX/level_crossing.h"
#include "STM/stm.h"
#include "Euroradio/terminal.h"
#include "platform_runtime.h"

#include <random>

#ifdef RADIO_CFM
#include "../EVC/Euroradio/tcp_cfm.h"
#include "console_platform.h"
#endif

void update();

void on_platform_ready()
{
#if RADIO_CFM
    initialize_cfm(dynamic_cast<ConsolePlatform&>(*platform).get_poller());
#endif

    platform->debug_print("Starting European Train Control System...");
    platform->on_quit_request().then([](){
        json odo(odometer_value);
        platform->debug_print(std::to_string(odometer_value));
        save_cold_data("odometer_value", odo);
        platform->quit();
    }).detach();

    std::random_device rd;
    nid_engine = rd() & 0xFFFFFFUL;

    start_dmi();
    start_or_iface();
    start_logging();
    initialize_mode_transitions();
    setup_stm_control();
    set_message_filters();
    initialize_national_functions();
    platform->delay(500).then(update).detach();
}
void update()
{
    update_or_iface();
    update_clock();
    update_odometer();
    update_geographical_position();
    update_track_comm();
    update_national_values();
    update_procedures();
    update_stm_control();
    update_lx();
    update_track_conditions();
    update_supervision();
    update_messages();
    update_national_functions();
    update_train_subsystems();
    update_dmi_windows();
    update_track_ahead_free_request();
    for (auto *session : active_sessions) {
        session->send_pending();
    }
    platform->delay(50).then(update).detach();
}

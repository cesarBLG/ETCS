/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "fixed_values.h"
#define TO_MPS(kph) kph/3.6
const double dV_ebi_min = TO_MPS(7.5);
const double dV_ebi_max = TO_MPS(15);
const double V_ebi_min = TO_MPS(110);
const double V_ebi_max = TO_MPS(210);

const double dV_sbi_min = TO_MPS(5.5);
const double dV_sbi_max = TO_MPS(10);
const double V_sbi_min = TO_MPS(110);
const double V_sbi_max = TO_MPS(210);

const double dV_warning_min = TO_MPS(4);
const double dV_warning_max = TO_MPS(5);
const double V_warning_min = TO_MPS(110);
const double V_warning_max = TO_MPS(140);

const double T_dispTTI = 14;
const double T_warning = 2;
const double T_driver = 4;
const double T_horn = 4;
const double T_delete_condition = 5;

const int M_rotating_max = 15;
const int M_rotating_min = 2;

const double T_CYCRQSTD = 60;
const double T_ACK = 5;

const double D_Metal = 300;

const double D_keep_information = 300;

const int N_tries_radio = 3;
const int N_message_repetition = 3;
const double T_message_repetition = 15;
const double T_keep_session = 300; 
const double T_connection_status = 45; 
const double T_network_registration = 40; 
const double T_disconnect_radio = 60;
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
#pragma once
extern const double dV_ebi_min;
extern const double dV_ebi_max;
extern const double V_ebi_min;
extern const double V_ebi_max;

extern const double dV_sbi_min;
extern const double dV_sbi_max;
extern const double V_sbi_min;
extern const double V_sbi_max;

extern const double dV_warning_min;
extern const double dV_warning_max;
extern const double V_warning_min;
extern const double V_warning_max;

extern const double T_dispTTI;
extern const double T_warning;
extern const double T_driver;
extern const double T_horn;
extern const double T_delete_condition;

extern const int M_rotating_max;
extern const int M_rotating_min;

extern const double T_CYCRQSTD;
extern const double T_ACK;

extern const double D_Metal;
 
extern const double D_keep_information;

extern const int N_tries_radio;
extern const int N_message_repetition;
extern const double T_message_repetition;
extern const double T_keep_session;
extern const double T_connection_status;
extern const double T_disconnect_radio;
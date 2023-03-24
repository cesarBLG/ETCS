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
#include "etcs_information.h"
struct national_values_information : etcs_information
{
    national_values_information() : etcs_information(0) {}
    void handle() override;
};
struct linking_information : etcs_information
{
    linking_information() : etcs_information(1) {}
    void handle() override;
};
struct signalling_information : etcs_information
{
    signalling_information() : etcs_information(2) {}
    void handle() override;
};
struct ma_information : etcs_information
{
    ma_information() : etcs_information(3) {}
    void handle() override;
};
struct ma_information_lv2 : etcs_information
{
    ma_information_lv2() : etcs_information(3) {}
    void handle() override;
};
struct repositioning_information : etcs_information
{
    repositioning_information() : etcs_information(4) {}
    void handle() override;
};
struct gradient_information : etcs_information
{
    gradient_information() : etcs_information(5) {}
    void handle() override;
};
struct issp_information : etcs_information
{
    issp_information() : etcs_information(6) {}
    void handle() override;
};
struct leveltr_order_information : etcs_information
{
    leveltr_order_information() : etcs_information(8, 10) {}
    void handle() override;
};
struct condleveltr_order_information : etcs_information
{
    condleveltr_order_information() : etcs_information(9, 10) {}
    void handle() override;
};
struct session_management_information : etcs_information
{
    session_management_information() : etcs_information(10, 11) {}
    void handle() override;
};
struct ma_request_params_info : etcs_information
{
    ma_request_params_info() : etcs_information(12, 13) {}
    void handle() override;
};
struct position_report_params_info : etcs_information
{
    position_report_params_info() : etcs_information(13, 14) {}
    void handle() override;
};
struct SR_authorisation_info : etcs_information
{
    SR_authorisation_info() : etcs_information(14,15) {}
    void handle() override;
};
struct stop_if_in_SR_information : etcs_information
{
    stop_if_in_SR_information() : etcs_information(15,16) {}
    void handle() override;
};
struct TSR_information : etcs_information
{
    TSR_information() : etcs_information(17,18) {}
    void handle() override;
};
struct TSR_revocation_information : etcs_information
{
    TSR_revocation_information() : etcs_information(18,19) {}
    void handle() override;
};
struct TSR_revocable_inhibition_information : etcs_information
{
    TSR_revocable_inhibition_information() : etcs_information(19,20) {}
    void handle() override;
};
struct TSR_gradient_information : etcs_information
{
    TSR_gradient_information() : etcs_information(20,21) {}
    void handle() override;
};
struct route_suitability_information : etcs_information
{
    route_suitability_information() : etcs_information(21,22) {}
    void handle() override;
};
struct plain_text_information : etcs_information
{
    plain_text_information() : etcs_information(23,24) {}
    void handle() override;
};
struct fixed_text_information : etcs_information
{
    fixed_text_information() : etcs_information(24,25) {}
    void handle() override;
};
struct geographical_position_information : etcs_information
{
    geographical_position_information() : etcs_information(25,26) {}
    void handle() override;
};
struct rbc_transition_information : etcs_information
{
    rbc_transition_information() : etcs_information(26,27) {}
    void handle() override;
};
struct danger_for_SH_information : etcs_information
{
    danger_for_SH_information() : etcs_information(27,28) {}
    void handle() override;
};
struct coordinate_system_information : etcs_information
{
    coordinate_system_information() : etcs_information(32, 33) {}
    void handle() override;
};
struct track_condition_information : etcs_information
{
    track_condition_information() : etcs_information(34,35) {}
    void handle() override;
};
struct track_condition_information2 : etcs_information
{
    track_condition_information2() : etcs_information(34,36) {}
    void handle() override;
};
struct track_condition_big_metal_information : etcs_information
{
    track_condition_big_metal_information() : etcs_information(35,37) {}
    void handle() override;
};
struct ma_shortening_information : etcs_information
{
    ma_shortening_information() : etcs_information(39,41) {}
    void handle() override;
};
struct SH_authorisation_info : etcs_information
{
    SH_authorisation_info() : etcs_information(44,46) {}
    void handle() override;
};
struct version_order_information : etcs_information
{
    version_order_information() : etcs_information(46,48) {}
    void handle() override;
};
struct train_running_number_information : etcs_information
{
    train_running_number_information() : etcs_information(48,50) {}
    void handle() override;
};
struct taf_level23_information : etcs_information
{
    taf_level23_information() : etcs_information(56,58) {}
    void handle() override;
};
struct pbd_information : etcs_information
{
    pbd_information() : etcs_information(57,59) {}
    void handle() override;
};
struct level_crossing_information : etcs_information
{
    level_crossing_information() : etcs_information(58,60) {}
    void handle() override;
};
struct vbc_order : etcs_information
{
    vbc_order() : etcs_information(59,61) {}
    void handle() override;
};
struct generic_ls_marker_information : etcs_information
{
    generic_ls_marker_information() : etcs_information(60,62) {}
    void handle() override;
};
struct lssma_display_on_information : etcs_information
{
    lssma_display_on_information() : etcs_information(61,63) {}
    void handle() override;
};
struct lssma_display_off_information : etcs_information
{
    lssma_display_off_information() : etcs_information(62,64) {}
    void handle() override;
};

/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
    linking_information() : etcs_information(1) {location_based=false;}
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
struct axle_load_speed_profile_information : etcs_information
{
    axle_load_speed_profile_information() : etcs_information(7) {}
    void handle() override;
};
struct leveltr_order_information : etcs_information
{
    leveltr_order_information() : etcs_information(8, 10) {location_based=false;}
    void handle() override;
};
struct condleveltr_order_information : etcs_information
{
    condleveltr_order_information() : etcs_information(9, 10) {location_based=false;}
    void handle() override;
};
struct session_management_information : etcs_information
{
    session_management_information() : etcs_information(10, 11) {location_based=false;}
    void handle() override;
};
struct ma_request_params_info : etcs_information
{
    ma_request_params_info() : etcs_information(12, 13) {location_based=false;}
    void handle() override;
};
struct position_report_params_info : etcs_information
{
    position_report_params_info() : etcs_information(13, 14) {}
    void handle() override;
};
struct SR_authorisation_info : etcs_information
{
    SR_authorisation_info() : etcs_information(14,15) {location_based=false;}
    void handle() override;
};
struct stop_if_in_SR_information : etcs_information
{
    stop_if_in_SR_information() : etcs_information(15,16) {location_based=false;}
    void handle() override;
};
struct TSR_information : etcs_information
{
    TSR_information() : etcs_information(17,18) {}
    void handle() override;
};
struct TSR_revocation_information : etcs_information
{
    TSR_revocation_information() : etcs_information(18,19) {location_based=false;}
    void handle() override;
};
struct TSR_revocable_inhibition_information : etcs_information
{
    TSR_revocable_inhibition_information() : etcs_information(19,20) {location_based=false;}
    void handle() override;
};
struct TSR_gradient_information : etcs_information
{
    TSR_gradient_information() : etcs_information(20,21) {location_based=false;}
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
    danger_for_SH_information() : etcs_information(27,28) {location_based=false;}
    void handle() override;
};
struct coordinate_system_information : etcs_information
{
    coordinate_system_information() : etcs_information(32, 33) {location_based=false;}
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
struct trip_exit_acknowledge_information : etcs_information
{
    trip_exit_acknowledge_information() : etcs_information(37,39) {location_based=false;}
    void handle() override;
};
struct ma_shortening_information : etcs_information
{
    ma_shortening_information() : etcs_information(39,41) {}
    void handle() override;
};
struct ces_information : etcs_information
{
    distance orig_position;
    ces_information(distance pos) : etcs_information(41,43), orig_position(pos) {}
    void handle() override;
};
struct SH_authorisation_info : etcs_information
{
    SH_authorisation_info() : etcs_information(44,46) {location_based=false;}
    void handle() override;
};
struct version_order_information : etcs_information
{
    version_order_information() : etcs_information(46,48) {location_based=false;}
    void handle() override;
};
struct taf_request_information : etcs_information
{
    taf_request_information() : etcs_information(47,49) {}
    void handle() override;
};
struct train_running_number_information : etcs_information
{
    train_running_number_information() : etcs_information(48,50) {location_based=false;}
    void handle() override;
};
struct reversing_area_information : etcs_information
{
    reversing_area_information() : etcs_information(53,55) {}
    void handle() override;
};
struct reversing_supervision_information : etcs_information
{
    reversing_supervision_information() : etcs_information(54,56) {location_based=false;}
    void handle() override;
};
struct default_balise_information : etcs_information
{
    default_balise_information() : etcs_information(55,57) {location_based=false;}
    void handle() override;
};
struct taf_level23_information : etcs_information
{
    taf_level23_information() : etcs_information(56,58) {location_based=false;}
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
    vbc_order() : etcs_information(59,61) {location_based=false;}
    void handle() override;
};
struct generic_ls_marker_information : etcs_information
{
    generic_ls_marker_information() : etcs_information(60,62) {location_based=false;}
    void handle() override;
};
struct lssma_display_on_information : etcs_information
{
    lssma_display_on_information() : etcs_information(61,63) {location_based=false;}
    void handle() override;
};
struct lssma_display_off_information : etcs_information
{
    lssma_display_off_information() : etcs_information(62,64) {location_based=false;}
    void handle() override;
};

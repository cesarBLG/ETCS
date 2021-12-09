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
#include "messages.h"
#include "3.h"
#include "12.h"
#include "15.h"
#include "16.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "65.h"
#include "66.h"
#include "67.h"
#include "68.h"
#include "69.h"
#include "72.h"
#include "79.h"
#include "80.h"
#include "88.h"
#include "132.h"
#include "136.h"
#include "137.h"
#include "../Supervision/national_values.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/override.h"
#include "../TrackConditions/track_condition.h"
#include "../DMI/text_message.h"
#include "../Position/geographical.h"
#include "../LX/level_crossing.h"
struct national_values_information : etcs_information
{
    national_values_information() : etcs_information(0) {}
    void handle() override
    {
        NationalValues nv = *(NationalValues*)linked_packets.front().get();
        national_values_received(nv, ref);
    }
};
struct linking_information : etcs_information
{
    linking_information() : etcs_information(1) {}
    void handle() override
    {
        Linking l = *(Linking*)linked_packets.front().get();
        update_linking(ref, l, infill, nid_bg);
    }
};
struct signalling_information : etcs_information
{
    signalling_information() : etcs_information(2) {}
    void handle() override
    {
        Level1_MA ma = *(Level1_MA*)linked_packets.front().get();
        if (ma.V_MAIN == 0) {
            if (!overrideProcedure && !infill)
                trigger_condition(18);
        } else {
            movement_authority newMA = movement_authority(ref, ma, timestamp);
            set_signalling_restriction(newMA, infill);
        }
    }
};
struct ma_information : etcs_information
{
    ma_information() : etcs_information(3) {}
    void handle() override
    {
        Level1_MA ma = *(Level1_MA*)linked_packets.front().get();
        movement_authority MA = movement_authority(ref, ma, timestamp);
        if (infill)
            MA_infill(MA);
        else
            replace_MA(MA);
        bool mp = false;
        for (auto it = ++linked_packets.begin(); it != linked_packets.end(); ++it) {
            if (it->get()->NID_PACKET == 80) {
                set_mode_profile(*(ModeProfile*)(it->get()), ref, infill);
                mp = true;
            }
        }
        if (!mp) reset_mode_profile(ref, infill);
    }
};
struct ma_information_lv2 : etcs_information
{
    ma_information_lv2() : etcs_information(3) {}
    void handle() override
    {
        Level2_3_MA ma = *(Level2_3_MA*)linked_packets.front().get();
        movement_authority MA = movement_authority(ref, ma, timestamp);
        if (infill)
            MA_infill(MA);
        else
            replace_MA(MA);
        bool mp = false;
        for (auto it = ++linked_packets.begin(); it != linked_packets.end(); ++it) {
            if (it->get()->NID_PACKET == 80) {
                set_mode_profile(*(ModeProfile*)(it->get()), ref, infill);
                mp = true;
            }
        }
        if (!mp) reset_mode_profile(ref, infill);
    }
};
struct repositioning_information : etcs_information
{
    repositioning_information() : etcs_information(4) {}
    void handle() override
    {
        RepositioningInformation ri = *(RepositioningInformation*)linked_packets.front().get();
        if (MA)
            MA->reposition(ref, ri.L_SECTION.get_value(ri.Q_SCALE));
    }
};
struct gradient_information : etcs_information
{
    gradient_information() : etcs_information(5) {}
    void handle() override
    {
        GradientProfile grad = *(GradientProfile*)linked_packets.front().get();
        std::map<distance, double> gradient;
        std::vector<GradientElement> elements;
        elements.push_back(grad.element);
        elements.insert(elements.end(), grad.elements.begin(), grad.elements.end());
        distance d = ref;
        for (auto e : elements) {
            d += e.D_GRADIENT.get_value(grad.Q_SCALE);
            gradient[d] = (e.Q_GDIR == Q_GDIR_t::Uphill ? 0.001 : -0.001)*e.G_A;
        }
        update_gradient(gradient);
    }
};
struct issp_information : etcs_information
{
    issp_information() : etcs_information(6) {}
    void handle() override
    {
        InternationalSSP issp = *(InternationalSSP*)linked_packets.front().get();
        update_SSP(get_SSP(ref, issp));
    }
};
struct leveltr_order_information : etcs_information
{
    leveltr_order_information() : etcs_information(8) {}
    void handle() override
    {
        LevelTransitionOrder LTO = *(LevelTransitionOrder*)linked_packets.front().get();
        level_transition_received(level_transition_information(LTO, ref));
    }
};
struct condleveltr_order_information : etcs_information
{
    condleveltr_order_information() : etcs_information(9) {}
    void handle() override
    {
        ConditionalLevelTransitionOrder CLTO = *(ConditionalLevelTransitionOrder*)linked_packets.front().get();
        level_transition_received(level_transition_information(CLTO, ref));
    }
};
struct stop_if_in_SR_information : etcs_information
{
    stop_if_in_SR_information() : etcs_information(15) {}
    void handle() override
    {
        StopIfInSR s = *(StopIfInSR*)linked_packets.front().get();
        // TODO: balises allowed
        if (s.Q_SRSTOP == Q_SRSTOP_t::StopIfInSR) {
            formerEoA = {};
            if (!overrideProcedure)
                trigger_condition(54);
        }
    }
};
struct TSR_information : etcs_information
{
    TSR_information() : etcs_information(17) {}
    void handle() override
    {
        TemporarySpeedRestriction t = *(TemporarySpeedRestriction*)linked_packets.front().get();
        std::vector<TSR_element_packet> elements;
        elements.push_back(t.element);
        elements.insert(elements.end(), t.elements.begin(), t.elements.end());
        distance start = ref;
        for (auto it = t.elements.begin(); it != t.elements.end(); ++it) {
            start += it->D_TSR.get_value(t.Q_SCALE);
            speed_restriction p(it->V_TSR.get_value(), start, start+it->L_TSR.get_value(t.Q_SCALE), it->Q_FRONT==Q_FRONT_t::TrainLengthDelay);
            TSR tsr = {tsr.id = it->NID_TSR, it->NID_TSR != NID_TSR_t::NonRevocable, p};
            insert_TSR(tsr);
        }
    }
};
struct TSR_revocation_information : etcs_information
{
    TSR_revocation_information() : etcs_information(18) {}
    void handle() override
    {
        TemporarySpeedRestrictionRevocation r = *(TemporarySpeedRestrictionRevocation*)linked_packets.front().get();
        revoke_TSR(r.NID_TSR);
    }
};
struct plain_text_information : etcs_information
{
    plain_text_information() : etcs_information(23) {}
    void handle() override
    {
        PlainTextMessage m = *(PlainTextMessage*)linked_packets.front().get();
        add_message(m, ref);
    }
};
struct geographical_position_information : etcs_information
{
    geographical_position_information() : etcs_information(25) {}
    void handle() override
    {
        GeographicalPosition m = *(GeographicalPosition*)linked_packets.front().get();
        handle_geographical_position(m, nid_bg);
    }
};
struct danger_for_SH_information : etcs_information
{
    danger_for_SH_information() : etcs_information(27) {}
    void handle() override
    {
        DangerForShunting s = *(DangerForShunting*)linked_packets.front().get();
        // TODO: balises allowed
        if (s.Q_ASPECT == Q_ASPECT_t::StopIfInSH) {
            if (!overrideProcedure)
                trigger_condition(49);
        }
    }
};
struct track_condition_information : etcs_information
{
    track_condition_information() : etcs_information(34,35) {}
    void handle() override
    {
        if (linked_packets.front()->NID_PACKET == 68) {
            TrackCondition tc = *(TrackCondition*)linked_packets.front().get();
            load_track_condition_various(tc, ref);
        } else if (linked_packets.front()->NID_PACKET == 69) {
            TrackConditionStationPlatforms tc = *(TrackConditionStationPlatforms*)linked_packets.front().get();
            load_track_condition_platforms(tc, ref);
        }
    }
};
struct track_condition_big_metal_information : etcs_information
{
    track_condition_big_metal_information() : etcs_information(35,37) {}
    void handle() override
    {
        TrackConditionBigMetalMasses tc = *(TrackConditionBigMetalMasses*)linked_packets.front().get();
        load_track_condition_bigmetal(tc, ref);
    }
};
struct level_crossing_information : etcs_information
{
    level_crossing_information() : etcs_information(58,60) {}
    void handle() override
    {
        LevelCrossingInformation lx = *(LevelCrossingInformation*)linked_packets.front().get();
        load_lx(lx, ref);
    }
};
void try_handle_information(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message);
std::vector<etcs_information*> construct_information(int packet_num);

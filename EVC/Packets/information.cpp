/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "information.h"
#include "messages.h"
#include "radio.h"
#include "vbc.h"
#include "2.h"
#include "3.h"
#include "6.h"
#include "12.h"
#include "15.h"
#include "16.h"
#include "21.h"
#include "27.h"
#include "39.h"
#include "40.h"
#include "41.h"
#include "42.h"
#include "49.h"
#include "51.h"
#include "52.h"
#include "57.h"
#include "58.h"
#include "63.h"
#include "65.h"
#include "66.h"
#include "67.h"
#include "68.h"
#include "69.h"
#include "70.h"
#include "72.h"
#include "76.h"
#include "79.h"
#include "80.h"
#include "88.h"
#include "90.h"
#include "131.h"
#include "132.h"
#include "136.h"
#include "137.h"
#include "138.h"
#include "139.h"
#include "140.h"
#include "141.h"
#include "180.h"
#include "TrainToTrack/9.h"
#include "../Supervision/emergency_stop.h"
#include "../Supervision/national_values.h"
#include "../Supervision/track_pbd.h"
#include "../Position/geographical.h"
#include "../LX/level_crossing.h"
#include "../Euroradio/session.h"
#include "../language/language.h"
#include "../Procedures/mode_transition.h"
#include "../Procedures/level_transition.h"
#include "../Procedures/override.h"
#include "../Procedures/reversing.h"
#include "../Procedures/stored_information.h"
#include "../Procedures/train_trip.h"
#include "../TrackConditions/track_condition.h"
#include "../TrackConditions/route_suitability.h"
#include "../DMI/text_message.h"
#include "../DMI/windows.h"
#include "../DMI/track_ahead_free.h"
#include "../Version/version.h"
void national_values_information::handle()
{
    NationalValues nv = *(NationalValues*)linked_packets.front().get();
    national_values_received(nv, *ref);
}
void linking_information::handle()
{
    Linking l = *(Linking*)linked_packets.front().get();
    update_linking(l, infill.has_value(), infill ? *infill : nid_bg);
}
void signalling_information::handle()
{
    Level1_MA ma = *(Level1_MA*)linked_packets.front().get();
    if (ma.V_MAIN == 0) {
        if (!overrideProcedure && !infill) {
            trigger_condition(18);
            trigger_condition(67);
        }
    } else {
        movement_authority newMA = movement_authority(*ref, ma, timestamp);
        set_signalling_restriction(newMA, infill.has_value());
    }
}
void ma_information::handle()
{
    Level1_MA ma = *(Level1_MA*)linked_packets.front().get();
    movement_authority MA = movement_authority(*ref, ma, timestamp);
    if (infill)
        MA_infill(MA);
    else
        replace_MA(MA);
    bool mp = false;
    sh_balises = {};
    for (auto it = ++linked_packets.begin(); it != linked_packets.end(); ++it) {
        if (it->get()->NID_PACKET == 49) {
            auto &balises = *((ListSRBalises*)it->get());
            sh_balises = std::set<bg_id>();
            int NID_C = nid_bg.NID_C;
            for (int i=0; i<balises.N_ITER; i++) {
                if (balises.elements[i].Q_NEWCOUNTRY)
                    NID_C = balises.elements[i].NID_C;
                sh_balises->insert({NID_C, (int)balises.elements[i].NID_BG});
            }
        } else if (it->get()->NID_PACKET == 80) {
            set_mode_profile(*(ModeProfile*)(it->get()), *ref, infill.has_value());
            mp = true;
        }
    }
    if (!mp) reset_mode_profile(*ref, infill.has_value());
}
void ma_information_lv2::handle()
{
    if (active_dialog == dialog_sequence::Main && active_dialog_step == "S7")
        active_dialog = dialog_sequence::None;
    Level2_3_MA ma = *(Level2_3_MA*)linked_packets.front().get();
    movement_authority MA = movement_authority(*ref, ma, timestamp);
    for (int i=0; i<5; i++) {
        ma_rq_reasons[i] = false;
    }
    if (infill)
        MA_infill(MA);
    else
        replace_MA(MA);
    bool mp = false;
    bool glsmk = false;
    sh_balises = {};
    for (auto it = ++linked_packets.begin(); it != linked_packets.end(); ++it) {
        if (it->get()->NID_PACKET == 49) {
            auto &balises = *((ListSRBalises*)it->get());
            sh_balises = std::set<bg_id>();
            int NID_C = nid_bg.NID_C;
            for (int i=0; i<balises.N_ITER; i++) {
                if (balises.elements[i].Q_NEWCOUNTRY)
                    NID_C = balises.elements[i].NID_C;
                sh_balises->insert({NID_C, (int)balises.elements[i].NID_BG});
            }
        } else if (it->get()->NID_PACKET == 80) {
            set_mode_profile(*(ModeProfile*)(it->get()), *ref, infill.has_value());
            mp = true;
        } else if (it->get()->NID_PACKET == 181) {
            glsmk = true;
        }
    }
    if (!glsmk) ls_function_marker = false;
    if (!mp) reset_mode_profile(*ref, infill.has_value());
}
void repositioning_information::handle()
{
    RepositioningInformation ri = *(RepositioningInformation*)linked_packets.front().get();
    if (MA)
        MA->reposition(*ref, ri.L_SECTION.get_value(ri.Q_SCALE));
}
void gradient_information::handle()
{
    GradientProfile grad = *(GradientProfile*)linked_packets.front().get();
    std::vector<std::pair<distance,double>> gradient;
    std::vector<GradientElement> elements;
    elements.push_back(grad.element);
    elements.insert(elements.end(), grad.elements.begin(), grad.elements.end());
    distance d = *ref;
    for (auto &e : elements) {
        d += e.D_GRADIENT.get_value(grad.Q_SCALE);
        if (e.G_A == e.G_A.EndOfGradient) {
            gradient.push_back({d, 1000});
            update_gradient(gradient);
            return;
        }
        gradient.push_back({d, (e.Q_GDIR == e.Q_GDIR.Uphill ? 0.001 : -0.001)*e.G_A});
    }
    gradient.push_back({distance::from_odometer(dist_base::max), 1000});
    update_gradient(gradient);
}
void issp_information::handle()
{
    InternationalSSP issp = *(InternationalSSP*)linked_packets.front().get();
    update_SSP(get_SSP(*ref, issp));
}
void axle_load_speed_profile_information::handle()
{
    AxleLoadSpeedProfile asp = *(AxleLoadSpeedProfile*)linked_packets.front().get();
    if (asp.Q_TRACKINIT == asp.Q_TRACKINIT.InitialState) {
        delete_ASP(*ref+asp.D_TRACKINIT.get_value(asp.Q_SCALE));
        return;
    }
    std::vector<ASP_element_packet> asps;
    asps.push_back(asp.element);
    asps.insert(asps.end(), asp.elements.begin(), asp.elements.end());
    std::vector<speed_restriction> rests;
    distance dist = *ref;
    for (auto &el : asps) {
        dist += el.D_AXLELOAD.get_value(asp.Q_SCALE);
        double v = 1000;
        for (auto &cat : el.diffs) {
            if ((int)axle_load_category >= cat.M_AXLELOADCAT)
                v = std::min(v, cat.V_AXLELOAD.get_value());
        }
        if (v < 1000)
            rests.push_back(speed_restriction(v, dist, dist+el.L_AXLELOAD.get_value(asp.Q_SCALE), el.Q_FRONT == el.Q_FRONT.TrainLengthDelay));
    }
    update_ASP(*ref+asps[0].D_AXLELOAD.get_value(asp.Q_SCALE), rests);
}
void leveltr_order_information::handle()
{
    LevelTransitionOrder LTO = *(LevelTransitionOrder*)linked_packets.front().get();
    level_transition_received(level_transition_information(LTO));
}
void condleveltr_order_information::handle()
{
    ConditionalLevelTransitionOrder CLTO = *(ConditionalLevelTransitionOrder*)linked_packets.front().get();
    level_transition_received(level_transition_information(CLTO));
}
void session_management_information::handle()
{
    SessionManagement &session = *(SessionManagement*)linked_packets.front().get();
    bool ignore = mode == Mode::SL && session.Q_SLEEPSESSION == session.Q_SLEEPSESSION.IgnoreOrder;
    contact_info info = {session.NID_C, session.NID_RBC, session.NID_RADIO};
    if (session.NID_RBC == session.NID_RBC.ContactLastRBC) info.id = ContactLastRBC;
    if (session.NID_RADIO == session.NID_RADIO.UseShortNumber) info.phone_number = UseShortNumber;
    if (session.Q_RBC == session.Q_RBC.EstablishSession) {
        set_supervising_rbc(info);
        if (supervising_rbc && mode != Mode::SH && mode != Mode::PS && !ignore)
            supervising_rbc->open(0);
    } else if (!ignore) {
        terminate_session(info);
    } 
}
void ma_request_params_info::handle()
{
    auto *params = (MovementAuthorityRequestParameters*)linked_packets.front().get();
    // TODO
}
void position_report_params_info::handle()
{
    auto *params = (PositionReportParameters*)linked_packets.front().get();
    position_report_parameters p;
    p.D_sendreport = params->D_CYCLOC.get_value(params->Q_SCALE);
    p.T_sendreport = params->T_CYCLOC * 1000;
    p.LRBG = params->M_LOC != params->M_LOC.NotEveryLRBG;
    distance pos = *ref;
    for (auto &e : params->elements) {
        pos += e.D_LOC.get_value(params->Q_SCALE);
        if (e.Q_LGTLOC == e.Q_LGTLOC.MaxSafeFrontEnd) p.location_front.push_back(pos);
        else p.location_rear.push_back(pos);
    }
    pos_report_params = p;
}
void SR_authorisation_info::handle()
{
    auto *sr = (SR_authorisation*)message->get();
    ma_rq_reasons[0] = false;
    if (sr->D_SR != sr->D_SR.Infinity)
        SR_dist_override = sr->D_SR.get_value(sr->Q_SCALE);
    else
        SR_dist_override = std::numeric_limits<double>::infinity();
    if (mode == Mode::SR) {
        SR_dist_start = distance::from_odometer(d_estfront_dir[odometer_orientation == -1]);
    } else {
        mode_to_ack = Mode::SR;
        mode_acknowledgeable = true;
        mode_acknowledged = false;
    }
    if (active_dialog == dialog_sequence::Main && active_dialog_step == "S7")
        active_dialog = dialog_sequence::None;
    sr_balises = {};
    for (auto &pack : sr->optional_packets) {
        if (pack->NID_PACKET == 63) {
            auto &balises = *((ListSRBalises*)pack.get());
            sr_balises = std::set<bg_id>();
            int NID_C = nid_bg.NID_C;
            for (int i=0; i<balises.N_ITER; i++) {
                if (balises.elements[i].Q_NEWCOUNTRY)
                    NID_C = balises.elements[i].NID_C;
                sr_balises->insert({NID_C, (int)balises.elements[i].NID_BG});
            }
        }
    }
}
void stop_if_in_SR_information::handle()
{
    StopIfInSR s = *(StopIfInSR*)linked_packets.front().get();
    if (s.Q_SRSTOP == s.Q_SRSTOP.StopIfInSR && (!sr_balises || sr_balises->find(nid_bg) == sr_balises->end() || (operated_version>>4)==1)) {
        if (!overrideProcedure)
            trigger_condition(54);
        else
            override_stopsr();
    }
}
void TSR_information::handle()
{
    TemporarySpeedRestriction t = *(TemporarySpeedRestriction*)linked_packets.front().get();
    distance start = *ref + t.D_TSR.get_value(t.Q_SCALE);
    speed_restriction p(t.V_TSR.get_value(), start, start+t.L_TSR.get_value(t.Q_SCALE), t.Q_FRONT == t.Q_FRONT.TrainLengthDelay);
    p.is_tsr = true;
    TSR tsr = {(int)t.NID_TSR, t.NID_TSR != t.NID_TSR.NonRevocable, p};
    insert_TSR(tsr);
}
void TSR_revocation_information::handle()
{
    TemporarySpeedRestrictionRevocation r = *(TemporarySpeedRestrictionRevocation*)linked_packets.front().get();
    revoke_TSR(r.NID_TSR);
}
void TSR_revocable_inhibition_information::handle()
{
    inhibit_revocable_tsr = true;
}
void TSR_gradient_information::handle()
{
    auto &gr = *(DefaultGradientTSR*)linked_packets.front().get();
    default_gradient_tsr = (gr.Q_GDIR == gr.Q_GDIR.Uphill ? 0.001 : -0.001)*gr.G_TSR;
    target::recalculate_all_decelerations();
}
void route_suitability_information::handle()
{
    auto &rs = *(RouteSuitabilityData*)linked_packets.front().get();
    load_route_suitability(rs, *ref);
}
void plain_text_information::handle()
{
    PlainTextMessage m = *(PlainTextMessage*)linked_packets.front().get();
    add_message(m, ref);
}
void fixed_text_information::handle()
{
    FixedTextMessage m = *(FixedTextMessage*)linked_packets.front().get();
    add_message(m, ref);
}
void geographical_position_information::handle()
{
    GeographicalPosition m = *(GeographicalPosition*)linked_packets.front().get();
    handle_geographical_position(m, nid_bg);
}
void rbc_transition_information::handle()
{
    RBCTransitionOrder o = *(RBCTransitionOrder*)linked_packets.front().get();
    contact_info info = {o.NID_C, o.NID_RBC, o.NID_RADIO};
    if (o.NID_RBC == o.NID_RBC.ContactLastRBC) info.id = ContactLastRBC;
    if (o.NID_RADIO == o.NID_RADIO.UseShortNumber) info.phone_number = UseShortNumber;
    if ((mode != Mode::SL || o.Q_SLEEPSESSION == o.Q_SLEEPSESSION.ExecuteOrder) && mode != Mode::PS && mode != Mode::SH) {
        rbc_handover(*ref + o.D_RBCTR.get_value(o.Q_SCALE), info);
    } else if (o.D_RBCTR == 0) {
        set_rbc_contact(info);
    }
}
void danger_for_SH_information::handle()
{
    DangerForShunting s = *(DangerForShunting*)linked_packets.front().get();
    if (s.Q_ASPECT == s.Q_ASPECT.StopIfInSH) {
        if (!overrideProcedure)
            trigger_condition(49);
        else
            override_stopsh();
    }
}
void coordinate_system_information::handle()
{
    auto &msg = *(coordinate_system_assignment*)message->get();
    auto bg = orbgs.end();
    auto prvbg = orbgs.end();
    bool prev_follows = false;
    for (auto it = orbgs.begin(); it != orbgs.end(); ++it) {
        if ((it->second & 1) == 0) {
            if (it->first.nid_lrbg == msg.NID_LRBG.get_value()) {
                bg = it;
                prev_follows = true;
            } else if (prev_follows) {
                if (prvbg != orbgs.end() && prvbg->first.nid_lrbg != it->first.nid_lrbg) {
                    prvbg = orbgs.end();
                    break;
                }
                if (prvbg == orbgs.end())
                    prvbg = it;
                prev_follows = false;
            }
        }
    }
    if (bg != orbgs.end() && prvbg != orbgs.end()) {
        if (prvbg->first.original_orientation == odometer_orientation)
            bg->first.dir = msg.Q_ORIENTATION == msg.Q_ORIENTATION.Reverse;
        else
            bg->first.dir = msg.Q_ORIENTATION != msg.Q_ORIENTATION.Reverse;
    }
}
void track_condition_information::handle()
{
    int nid = linked_packets.front()->NID_PACKET;
    if (nid == 68) {
        TrackCondition &tc = *(TrackCondition*)linked_packets.front().get();
        load_track_condition_various(tc, *ref, false);
    } else if (nid == 69) {
        TrackConditionStationPlatforms &tc = *(TrackConditionStationPlatforms*)linked_packets.front().get();
        load_track_condition_platforms(tc, *ref);
    } else if (nid == 39) {
        TrackConditionChangeTractionSystem &tc = *(TrackConditionChangeTractionSystem*)linked_packets.front().get();
        load_track_condition_traction(tc, *ref);
    } else if (nid == 40) {
        TrackConditionChangeCurrentConsumption &tc = *(TrackConditionChangeCurrentConsumption*)linked_packets.front().get();
    }
}
void track_condition_information2::handle()
{
    TrackCondition &tc = *(TrackCondition*)linked_packets.front().get();
    load_track_condition_various(tc, *ref, true);
}
void track_condition_big_metal_information::handle()
{
    TrackConditionBigMetalMasses tc = *(TrackConditionBigMetalMasses*)linked_packets.front().get();
    load_track_condition_bigmetal(tc, *ref);
}
void trip_exit_acknowledge_information::handle()
{
    trip_exit_acknowledged = true;
    trip_exit_acknowledge_timestamp = timestamp;
}
void ma_shortening_information::handle()
{
    Level2_3_MA ma = *(Level2_3_MA*)linked_packets.front().get();
    movement_authority MA = movement_authority(*ref, ma, timestamp);
    MA.calculate_distances();
    bool accept = true;
    if (MA.LoA_ma || MA.SvL_ma) {
        target svl(MA.LoA_ma ? MA.LoA_ma->first.max : MA.SvL_ma->max, MA.LoA_ma ? MA.LoA_ma->second : 0, MA.LoA_ma ? target_class::LoA : target_class::SvL);
        svl.calculate_curves(V_est, A_est, V_ura);
        if (svl.d_I < d_maxsafefront(svl.get_target_position()))
            accept = false;
    }
    if (MA.EoA_ma) {
        target eoa(MA.EoA_ma->est, 0, target_class::EoA);
        eoa.calculate_curves(V_est, A_est, V_ura);
        if (eoa.d_I < d_estfront)
            accept = false;
    }
    if (accept) {
        replace_MA(MA, true);
        bool mp = false;
        sh_balises = {};
        for (auto it = ++linked_packets.begin(); it != linked_packets.end(); ++it) {
            if (it->get()->NID_PACKET == 49) {
                auto &balises = *((ListSRBalises*)it->get());
                sh_balises = std::set<bg_id>();
                int NID_C = nid_bg.NID_C;
                for (int i=0; i<balises.N_ITER; i++) {
                    if (balises.elements[i].Q_NEWCOUNTRY)
                        NID_C = balises.elements[i].NID_C;
                    sh_balises->insert({NID_C, (int)balises.elements[i].NID_BG});
                }
            } else if (it->get()->NID_PACKET == 80) {
                set_mode_profile(*(ModeProfile*)(it->get()), *ref, infill.has_value());
                mp = true;
            }
        }
        if (!mp) reset_mode_profile(*ref, infill.has_value());
        svl_shorten('f');
        if (supervising_rbc) {
            auto *msg = new ma_shorten_granted();
            msg->T_TRAINreq.rawdata = message->get()->T_TRAIN.rawdata;
            supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(msg));
        }
    } else if (supervising_rbc) {
        auto *msg = new ma_shorten_rejected();
        msg->T_TRAINreq.rawdata = message->get()->T_TRAIN.rawdata;
        supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(msg));
    }
}
void ces_information::handle()
{
    auto *emerg = (conditional_emergency_stop*)message->get();
    int result = handle_conditional_emergency_stop(emerg->NID_EM, *ref+emerg->D_EMERGENCYSTOP.get_value(emerg->Q_SCALE), d_minsafe(orig_position.est, confidence_data::from_distance(ref->est)));
    emergency_acknowledgement_message *ack = new emergency_acknowledgement_message();
    ack->NID_EM = emerg->NID_EM;
    ack->Q_EMERGENCYSTOP.rawdata = result;
    supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(ack));
}
void SH_authorisation_info::handle()
{
    update_dialog_step("SH authorised", "");
    sh_balises = {};
    for (auto it = linked_packets.begin(); it != linked_packets.end(); ++it) {
        if (it->get()->NID_PACKET == 49) {
            auto &balises = *((ListSRBalises*)it->get());
            sh_balises = std::set<bg_id>();
            int NID_C = nid_bg.NID_C;
            for (int i=0; i<balises.N_ITER; i++) {
                if (balises.elements[i].Q_NEWCOUNTRY)
                    NID_C = balises.elements[i].NID_C;
                sh_balises->insert({NID_C, (int)balises.elements[i].NID_BG});
            }
        }
    }
}
void version_order_information::handle()
{
    auto &v = *(SystemVersionOrder*)linked_packets.front().get();
    operate_version(v.M_VERSION, false);
}
void taf_request_information::handle()
{
    auto *taf = (taf_request_message*)message->get();
    distance dist = *ref + taf->D_TAFDISPLAY.get_value(taf->Q_SCALE);
    double length = taf->L_TAFDISPLAY.get_value(taf->Q_SCALE);
    request_track_ahead_free(dist, length);
}
void train_running_number_information::handle()
{
    auto &trn = *(TrainRunningNumberRBC*)linked_packets.front().get();
    train_running_number = 0;
    for (int i=7; i>=0; i--) {
        int hex = trn.NID_OPERATIONAL.rawdata>>(4*i);
        if (hex < 10)
            train_running_number = train_running_number*10+hex;
    }
    train_running_number_valid = true;
}
void reversing_area_information::handle()
{
    rv_position = {};
    auto &rai = *(ReversingAreaInformation*)linked_packets.front().get();
    rv_area = {*ref+rai.D_STARTREVERSEAREA.get_value(rai.Q_SCALE), *ref+rai.D_STARTREVERSEAREA.get_value(rai.Q_SCALE)+rai.L_REVERSEAREA.get_value(rai.Q_SCALE)};
    rv_area_end_original = rv_area->end;
}
void reversing_supervision_information::handle()
{
    rv_position = {};
    auto &rsi = *(ReversingSupervisionInformation*)linked_packets.front().get();
    rv_supervision = {rsi.V_REVERSE.get_value(), rsi.D_REVERSE.get_value(rsi.Q_SCALE)};
}
void default_balise_information::handle()
{
    int64_t time = get_milliseconds();
    add_message(text_message(get_text("Trackside malfunction"), true, false, false, [time](text_message& t) { return time + 30000 < get_milliseconds(); }));
}
void taf_level23_information::handle()
{
    if (supervising_rbc && supervising_rbc->status == session_status::Established) {
        auto &taf = *(TrackAheadFreeTransition*)linked_packets.front().get();
        auto *lti = new Level23TransitionInformation();
        lti->NID_LTRBG.set_value(bg_id({taf.Q_NEWCOUNTRY ? (int)taf.NID_C : (int)nid_bg.NID_C, (int)taf.NID_BG}));
        auto pack = std::shared_ptr<ETCS_packet>(lti);
        auto *req = new MA_request();
        req->Q_MARQSTREASON.rawdata = req->Q_MARQSTREASON.TrackAheadFreeBit;
        req->optional_packets.push_back(pack);
        supervising_rbc->queue(std::shared_ptr<euroradio_message_traintotrack>(req));
    }
}
void pbd_information::handle()
{
    auto &pbd = *(PermittedBrakingDistanceInformation*)linked_packets.front().get();
    load_PBD(pbd, *ref);
}
void level_crossing_information::handle()
{
    auto &lx = *(LevelCrossingInformation*)linked_packets.front().get();
    load_lx(lx, *ref);
}
void vbc_order::handle()
{
    VirtualBaliseCoverOrder &vbco = *(VirtualBaliseCoverOrder*)linked_packets.front().get();
    virtual_balise_cover vbc = {(int)vbco.NID_C, (int)vbco.NID_VBCMK, timestamp, vbco.T_VBC.get_value()};
    if (vbco.Q_VBCO == vbco.Q_VBCO.SetVBC)
        set_vbc(vbc);
    else
        remove_vbc(vbc);
}
void generic_ls_marker_information::handle()
{
    ls_function_marker = true;
}
void lssma_display_on_information::handle()
{
    auto *disp = (LSSMAToggleOrder*)linked_packets.front().get();
    display_lssma_time = disp->T_LSSMA.rawdata + (reevaluated ? get_milliseconds() : timestamp);
}
void lssma_display_off_information::handle()
{
    display_lssma_time = {};
}
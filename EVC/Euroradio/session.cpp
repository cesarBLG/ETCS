/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "session.h"
#include "../Packets/messages.h"
#include "../Packets/logging.h"
#include "../Packets/TrainToTrack/4.h"
#include "../Packets/TrainToTrack/5.h"
#include "../Supervision/train_data.h"
#include "../Procedures/start.h"
#include "../Procedures/level_transition.h"
#include "../TrainSubsystems/cold_movement.h"
#include "../TrackConditions/track_condition.h"
#include "../language/language.h"
#include "../Version/version.h"
#include "../Version/translate.h"
#include "platform_runtime.h"
#include "cfm.h"
#include <map>
communication_session *supervising_rbc = nullptr;
communication_session *accepting_rbc = nullptr;
communication_session *handing_over_rbc = nullptr;
bool handover_report_accepting = false;
bool handover_report_min = false;
bool handover_report_max = false;
distance rbc_transition_position;
safe_radio_status radio_status_driver;
std::set<communication_session*> active_sessions;
bool radio_hole = false;
void communication_session::open(int ntries)
{
    pending_ack.remove_if([](const msg_expecting_ack &mack){return mack.nid_ack.find(-1) != mack.nid_ack.end();});     
    if (status != session_status::Inactive)
        return;
    train_data_ack_pending = train_data_valid;
    train_data_ack_sent = false;
    train_running_number_sent = false;
    accept_unknown_position = true;
    prvlrbgs.clear();
    closing = false;
    initsent = false;
    status = session_status::Establishing;
    radio_status = safe_radio_status::Disconnected;
    connection = nullptr;
    for (auto *session : active_sessions) {
        if (session != this && session != accepting_rbc && session != handing_over_rbc && session != supervising_rbc)
            session->close();
    }
    last_active = get_milliseconds();
    connection_timer = true;
    tried = 0;
    this->ntries = ntries;
}
void communication_session::close()
{
    if (closing)
        return;
    if (status != session_status::Established) {
        finalize();
        return;
    }
    closing = true;
    pending_ack.clear();
    auto terminate = std::shared_ptr<euroradio_message_traintotrack>(new terminate_communication_session());
    send(terminate);
}
void communication_session::finalize()
{
    status = session_status::Inactive;
    if (radio_status == safe_radio_status::Connected)
        radio_status = safe_radio_status::Disconnected;
    if (connection) {
        connection->release();
        connection = nullptr;
    }
    pending_errors.clear();
    rx_list.clear();
    closing = false;
}
void communication_session::message_received(std::shared_ptr<euroradio_message> msg)
{
    log_message(*msg, d_estfront, get_milliseconds());
    if (!msg->valid || msg->readerror) {
        report_error(3);
#ifdef DEBUG_MSG_CONSISTENCY
        platform->debug_print("Message rejected: consistency error");
#endif
        return;
    }
    if (closing && msg->NID_MESSAGE != 39) {
#ifdef DEBUG_MSG_CONSISTENCY
        platform->debug_print("Message rejected");
#endif
        return;
    }
    int64_t timestamp = msg->T_TRAIN.get_value();
    if (timestamp <= last_valid_timestamp) {
        if (msg->NID_MESSAGE != 15 && msg->NID_MESSAGE != 16) {
            report_error(4);
#ifdef DEBUG_MSG_CONSISTENCY
            platform->debug_print("Message rejected: T_TRAIN < last_valid_timestamp");
#endif
            return;
        }
    } else {
        last_valid_timestamp = timestamp;
    }
    if (msg->NID_MESSAGE == 32 || msg->NID_MESSAGE == 39)
        pending_ack.remove_if([msg](const msg_expecting_ack &mack){return mack.nid_ack.find(msg->NID_MESSAGE) != mack.nid_ack.end();});
    if (msg->NID_MESSAGE == 32) {
        status = session_status::Established;
        if (!som_active)
            position_report_reasons[7] = true;
        auto rbc_ver = (RBC_version*)msg.get();
        version = rbc_ver->M_VERSION;
        if (is_version_supported(version)) {
            auto established = std::shared_ptr<communication_session_established>(new communication_session_established());
            auto &ver = established->SupportedVersions;
            for (auto it = supported_versions.begin(); it != supported_versions.end(); ++it) {
                M_VERSION_t M_VERSION;
                M_VERSION.rawdata = *it;
                if (it == supported_versions.begin())
                    ver->M_VERSION = M_VERSION;
                else
                    ver->M_VERSIONs.push_back(M_VERSION);
            }
            ver->N_ITER.rawdata = supported_versions.size() - 1;
            send(established);
        } else {
            auto nover = std::shared_ptr<no_compatible_session_supported>(new no_compatible_session_supported());
            send(nover);
            close();
            int64_t time = get_milliseconds();
            text_message msg(get_text("Trackside not compatible"), true, false, 2, [time](text_message &msg) {
                return time+30000<get_milliseconds();
            });
            add_message(msg);
        }
    } else if (msg->NID_MESSAGE == 39) {
        finalize();
    }
    if (msg->M_ACK == msg->M_ACK.AcknowledgementRequired) {
        auto ack = std::shared_ptr<acknowledgement_message>(new acknowledgement_message());
        ack->T_TRAINack = msg->T_TRAIN;
        send(ack);
    }
    bool success = handle_radio_message(msg, this);
    if (success && msg->NID_MESSAGE != 32 && msg->NID_MESSAGE != 39)
        pending_ack.remove_if([msg](const msg_expecting_ack &mack){return mack.nid_ack.find(msg->NID_MESSAGE) != mack.nid_ack.end();});
    update_ack();
}
void communication_session::report_error(int error)
{
    if (!isRBC) return;
    pending_errors.insert(error);
}
void communication_session::update_ack()
{
    for (auto it = pending_ack.begin(); it!=pending_ack.end(); ++it) {
        auto &msg = *it;
        if (get_milliseconds()-msg.last_sent > T_message_repetition*1000) {
            if (msg.times_sent > N_message_repetition) {
                if (msg.message->NID_MESSAGE == 155 || msg.message->NID_MESSAGE == 156) {
                    finalize();
                } else {
                    close();
                }
                break;
            } else {
                msg.times_sent++;
                msg.last_sent = get_milliseconds();
                fill_message(msg.message.get());
                if (msg.message->PositionReport2BG && msg.message->PositionReport2BG->get()->NID_PRVLRBG.rawdata != msg.message->PositionReport2BG->get()->NID_PRVLRBG.Unknown) {
                    prvlrbgs[msg.message->PositionReport2BG->get()->NID_LRBG.get_value()].insert(msg.message->PositionReport2BG->get()->NID_PRVLRBG.get_value());
                }
                if (VERSION_X(version) == 1) {
                    if (msg.message->PositionReport1BG) {
                        auto &mode = msg.message->PositionReport1BG->get()->M_MODE;
                        if (mode == 15)
                            mode.rawdata = 3;
                    }
                    if (msg.message->PositionReport2BG) {
                        auto &mode = msg.message->PositionReport2BG->get()->M_MODE;
                        if (mode == 15)
                            mode.rawdata = 3;
                    }
                }
                log_message(*msg.message, d_estfront, get_milliseconds());
                if (connection != nullptr) {
                    connection->send(msg.message);
                }
            }
        }
    }
}
void communication_session::update()
{
    if (status == session_status::Inactive) {
        connection_timer = false;
        return;
    }
    if (connection != nullptr)
        radio_status = connection->status;
    else if (radio_status == safe_radio_status::Connected)
        radio_status = safe_radio_status::Failed;
    if (radio_status == safe_radio_status::Connected) {
        last_active = get_milliseconds();
        connection_timer = false;
    }
    if (!isRBC && level != Level::N1 && !closing) {
        close();
    }

    bool radio_powered = false;
    for (auto *t : mobile_terminals) {
        if (t->powered) {
            radio_powered = true;
            break;
        }
    }
    bool radio = radio_powered && !radio_hole;

    if (status == session_status::Establishing) {
        if (!radio) {
            finalize();
        } else if (tried == 0 || ((tried<ntries || ntries <= 0) && radio_status == safe_radio_status::Failed)) {
            setup_connection();
        } else if (tried >= ntries && ntries > 0 && radio_status == safe_radio_status::Failed) {
            status = session_status::Inactive;
            connection = nullptr;
        }
        if (radio_status == safe_radio_status::Connected && !initsent) {
            auto init = std::shared_ptr<euroradio_message_traintotrack>(new init_communication_session());
            send(init);
            initsent = true;
        }
    } else if (status == session_status::Established) {
        if (radio_status == safe_radio_status::Failed) {
            connection_timer = true;
            if (train_data_ack_pending && train_data_ack_sent)
                train_data_ack_sent = false;
            if (radio) {
                if (get_milliseconds()-last_active > T_keep_session * 1000) {
                    auto ack = pending_ack;
                    finalize();
                    open(0);
                    pending_ack = ack;
                } else {
                    setup_connection();
                }
            }
        } else if (!closing) {    
            if ((train_data_valid && train_data_ack_pending && !train_data_ack_sent) || (VERSION_X(version) == 1 && train_running_number_valid && !train_running_number_sent)) {
                train_data_ack_sent = true;
                train_running_number_sent = true;
                auto *tdm = new validated_train_data_message();
                auto *td = new TrainDataPacket();
                td->NC_CDTRAIN.set_value(cant_deficiency);
                td->NC_TRAIN.rawdata = 0;
                for (int t : other_train_categories) {
                    td->NC_TRAIN.rawdata |= 1<<t;
                }
                td->L_TRAIN.set_value(L_TRAIN);
                td->V_MAXTRAIN.set_value(V_train);
                td->N_AXLE.rawdata = axle_number;
                td->M_AIRTIGHT.rawdata = Q_airtight ? td->M_AIRTIGHT.Fitted : td->M_AIRTIGHT.NotFitted;
                tdm->TrainData = std::shared_ptr<TrainDataPacket>(td);
                if (VERSION_X(version) != 1) {
                    auto *trn = new TrainRunningNumber();
                    trn->NID_OPERATIONAL.rawdata = 0;
                    int tmp = train_running_number;
                    for (int i=0; i<8; i++) {
                        trn->NID_OPERATIONAL.rawdata |= (tmp % 10)<<(4*i);
                        tmp /= 10;
                    }
                    tdm->optional_packets.push_back(std::shared_ptr<TrainRunningNumber>(trn));
                }
                queue(std::shared_ptr<validated_train_data_message>(tdm));
            }
            if (!train_running_number_sent && train_running_number_valid) {
                train_running_number_sent = true;
                auto *rep = new position_report();
                auto *trn = new TrainRunningNumber();
                trn->NID_OPERATIONAL.rawdata = 0;
                int tmp = train_running_number;
                for (int i=0; i<8; i++) {
                    trn->NID_OPERATIONAL.rawdata |= (tmp % 10)<<(4*i);
                    tmp /= 10;
                }
                rep->optional_packets.push_back(std::shared_ptr<TrainRunningNumber>(trn));
                queue(std::shared_ptr<euroradio_message_traintotrack>(rep));
            }
            if (!pending_errors.empty()) {
                auto *rep = new position_report();
                auto msg = std::shared_ptr<euroradio_message_traintotrack>(rep);
                for (int error : pending_errors) {
                    auto err = std::make_shared<ErrorReporting>();
                    err->M_ERROR.rawdata = error;
                    msg->optional_packets.push_back(err);
                }
                pending_errors.clear();
                queue(msg);
            }
        }
    }
    if (!rx_list.empty()) {
        auto tmp_list = std::move(rx_list);
        rx_list.clear();
        for (auto &msg : tmp_list) {
            message_received(msg);
        }
    }
    update_ack();
}

void communication_session::setup_connection()
{
    #if RADIO_CFM
        connection = std::make_unique<safe_radio_connection>(this);
    #else
        connection = std::make_unique<bus_radio_connection>(this);
    #endif
        connection->Sa_connect_request({{isRBC ? 1u : 0u, (contact.country<<14)|contact.id}, RadioNetworkId, contact.phone_number}, {2, 0});
        tried++;
}

void communication_session::queue(std::shared_ptr<euroradio_message_traintotrack> msg)
{
    if (status == session_status::Inactive || (status == session_status::Establishing && msg->NID_MESSAGE != 155) || (closing && msg->NID_MESSAGE != 156))
        return;
    if (msg->NID_MESSAGE == 136) {
        for (auto &msg2 : tx_list) {
            if (msg2->NID_MESSAGE == 136) {
                msg2->optional_packets.insert(msg2->optional_packets.end(), msg->optional_packets.begin(), msg->optional_packets.end());
                return;
            }
        }
    }
    tx_list.push_back(msg);
}
void communication_session::send(std::shared_ptr<euroradio_message_traintotrack> msg)
{
    fill_message(msg.get());
    if (msg->PositionReport2BG && msg->PositionReport2BG->get()->NID_PRVLRBG.rawdata != msg->PositionReport2BG->get()->NID_PRVLRBG.Unknown) {
        prvlrbgs[msg->PositionReport2BG->get()->NID_LRBG.get_value()].insert(msg->PositionReport2BG->get()->NID_PRVLRBG.get_value());
    }
    msg = translate_message(msg, version);
    log_message(*msg, d_estfront, get_milliseconds());
    if (status == session_status::Inactive || (status == session_status::Establishing && msg->NID_MESSAGE != 155) || (closing && msg->NID_MESSAGE != 156))
        return;
    std::set<int> ack;
    if (msg->NID_MESSAGE == 129)
        ack = {8};
    else if (msg->NID_MESSAGE == 130)
        ack = {27, 28};
    else if (msg->NID_MESSAGE == 132 && ((((MA_request*)msg.get())->Q_MARQSTREASON>>((MA_request*)msg.get())->Q_MARQSTREASON.StartSelectedByDriverBit) & 1) == 1)
        ack = {2, 3, 33};
    else if (msg->NID_MESSAGE == 150)
        ack = {-1};
    else if (msg->NID_MESSAGE == 155)
        ack = {32};
    else if (msg->NID_MESSAGE == 156)
        ack = {39};
    else if (msg->NID_MESSAGE == 157 && ((SoM_position_report*)msg.get())->Q_STATUS != ((SoM_position_report*)msg.get())->Q_STATUS.Valid)
        ack = {40,41,43};
    if (!ack.empty()) {
        pending_ack.remove_if([msg](const msg_expecting_ack &mack){return mack.message->NID_MESSAGE == msg->NID_MESSAGE;});
        pending_ack.push_back({ack,msg,1,get_milliseconds()});
    }
    if (radio_status == safe_radio_status::Connected && connection != nullptr) {
        connection->send(msg);
    }
}
void communication_session::send_pending()
{
    for (auto &msg : tx_list) {
        send(msg);
    }
    tx_list.clear();
}
int64_t first_supervised_timestamp;
bool radio_reaction_applied = false;
bool radio_reaction_reconnected = false;
void update_euroradio()
{
    if (supervising_rbc && supervising_rbc->status == session_status::Established && mode == Mode::IS)
        supervising_rbc->close();

    update_cfm();
    for (auto it = active_sessions.begin(); it != active_sessions.end(); ) {
        if ((*it)->status == session_status::Inactive && *it != supervising_rbc && *it != accepting_rbc && *it != handing_over_rbc) {
            delete *it;
            it = active_sessions.erase(it);
        } else {
            (*it)->update();
            ++it;
        }
    }
    if (accepting_rbc && accepting_rbc->status == session_status::Inactive) {
        accepting_rbc = handing_over_rbc = nullptr;
        transition_buffer.clear();
    }
    if (handing_over_rbc && handing_over_rbc->status == session_status::Inactive)
        handing_over_rbc = nullptr;
    if (accepting_rbc && d_maxsafefront(rbc_transition_position) < rbc_transition_position.max) {
        if (!handover_report_accepting && accepting_rbc->status == session_status::Established) {
            handover_report_accepting = true;
            handover_report_max = true;
            position_report_reasons[10] = true;
            supervising_rbc = accepting_rbc;
            accepting_rbc = nullptr;
            set_rbc_contact(supervising_rbc->contact);
            inhibit_revocable_tsr = false;
            for (auto it=transition_buffer.begin(); it!=transition_buffer.end(); ++it) {
                for (auto it2 = it->begin(); it2!=it->end(); ++it2) {
                    try_handle_information(*it2, *it);
                }
            }
            transition_buffer.clear();
        } else if (!handover_report_max) {
            handover_report_max = true;
            position_report_reasons[10] = true;
        }
        if (handing_over_rbc && handing_over_rbc->status == session_status::Establishing)
            handing_over_rbc->finalize();
    }
    if (handing_over_rbc && d_minsafefront(rbc_transition_position) - L_TRAIN < rbc_transition_position.min && !handover_report_min) {
        position_report_reasons[4] = true;
        handover_report_min = true;
    }
    if (supervising_rbc) {
        bool c1 = supervising_rbc->status == session_status::Inactive && supervising_rbc->radio_status == safe_radio_status::Failed && som_active;
        bool c2 = supervising_rbc->connection_timer && get_milliseconds()-supervising_rbc->last_active>T_connection_status*1000;
        bool c3 = supervising_rbc->status == session_status::Inactive && supervising_rbc->radio_status == safe_radio_status::Failed && !som_active;
        bool c4 = supervising_rbc->radio_status == safe_radio_status::Connected;
        bool c5 = supervising_rbc->status == session_status::Inactive && supervising_rbc->radio_status == safe_radio_status::Disconnected;
        bool c6 = supervising_rbc->status == session_status::Inactive && supervising_rbc->radio_status == safe_radio_status::Failed;
        bool c7 = supervising_rbc->radio_status == safe_radio_status::Failed && false;
        if (c1 && radio_status_driver == safe_radio_status::Disconnected)
            radio_status_driver = safe_radio_status::Failed;
        else if (c2)
            radio_status_driver = safe_radio_status::Failed;
        else if (c3 && radio_status_driver == safe_radio_status::Failed)
            radio_status_driver = safe_radio_status::Disconnected;
        else if (c4)
            radio_status_driver = safe_radio_status::Connected;
        else if ((c5 || c6 || c7) && radio_status_driver == safe_radio_status::Connected)
            radio_status_driver = safe_radio_status::Disconnected;
    } else {
        radio_status_driver = safe_radio_status::Disconnected;
    }
    bool prev_radio_hole = radio_hole;
    radio_hole = false;
    for (auto it = track_conditions.begin(); it != track_conditions.end(); ++it) {
        auto *tc = it->get();
        if (tc->condition == TrackConditions::RadioHole) {
            if (tc->start.max < d_maxsafefront(tc->start) && tc->end.min > d_minsafefront(tc->start) - L_TRAIN)
                radio_hole = true;
        }
    }
    if (!radio_hole && (level == Level::N2 || level == Level::N3) && (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS)) {
        int64_t supervised_timestamp = first_supervised_timestamp;
        if (supervising_rbc && supervising_rbc->last_valid_timestamp > first_supervised_timestamp)
            supervised_timestamp = supervising_rbc->last_valid_timestamp;
        if (get_milliseconds() - supervised_timestamp > T_NVCONTACT*1000 && !radio_reaction_applied) {
            radio_reaction_applied = true;
            switch (M_NVCONTACT) {
                case 2:
                    break;
                case 1:
                    trigger_brake_reason(2);
                    break;
                default:
                    trigger_condition(41);
                    break;
            }
            if (supervising_rbc)
                supervising_rbc->report_error(5);
        }
        if (supervising_rbc && get_milliseconds() < supervising_rbc->last_valid_timestamp + T_NVCONTACT*1000) {
            radio_reaction_applied = false;
            radio_reaction_reconnected = false;
        }
        if (get_milliseconds() - supervised_timestamp > (T_NVCONTACT + T_disconnect_radio)*1000 && !radio_reaction_reconnected) {
            radio_reaction_reconnected = true;
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                supervising_rbc->reset_radio();
        }
    } else {
        if (radio_hole || (level != Level::N2 && level != Level::N3 && (!supervising_rbc || supervising_rbc->status != session_status::Established)))
            first_supervised_timestamp = get_milliseconds();
        radio_reaction_applied = false;
        radio_reaction_reconnected = false;
    }
    if (prev_radio_hole && !radio_hole) {
        for (auto *session : active_sessions) {
            if (session->status == session_status::Inactive)
                session->open(0);
        }
    }
    if (supervising_rbc && supervising_rbc->status == session_status::Established && (level == Level::N2 || level == Level::N3))
        operate_version(supervising_rbc->version, true);
}
optional<contact_info> rbc_contact;
bool rbc_contact_valid;
void load_contact_info()
{
    //TODO: Radio Network
    json j = load_cold_data("RBCData");
    if (!j.is_null()) {
        rbc_contact = {j["NID_C"].get<unsigned int>(), j["NID_RBC"].get<unsigned int>(), j["PhoneNumber"]};
#if SIMRAIL
        // During takeover the train in SimRail multiplayer, the odometer is Unknown (due to spawning a copy of train, but the train is in move, so prepared RBC data hase to be always valid here)
        rbc_contact_valid = rbc_contact.has_value() && rbc_contact.value().id != 0x3FFF;
#else
        rbc_contact_valid = cold_movement_status == NoColdMovement;
#endif
    } else {
        rbc_contact_valid = false;
        rbc_contact = {};
    }
    j = load_cold_data("RadioNetworkId");
    if (!j.is_null())
        RadioNetworkId = j;
}
void set_rbc_contact(contact_info contact)
{
    rbc_contact = contact;
    rbc_contact_valid = contact.id != 0x3FFF;
    json j;
    j["NID_C"] = rbc_contact->country;
    j["NID_RBC"] = rbc_contact->id;
    j["PhoneNumber"] = rbc_contact->phone_number;
    save_cold_data("RBCData", j);
}
void set_supervising_rbc(contact_info info)
{
    if (accepting_rbc != nullptr) {
        transition_buffer = {};
    }
    handing_over_rbc = accepting_rbc = nullptr;
    handover_report_accepting = handover_report_max = handover_report_min = false;
    if (info.id == ContactLastRBC) {
        if (rbc_contact)
            info = *rbc_contact;
        else
            return;
    } else if (info.phone_number == UseShortNumber) {
        if (info.country == 0 && info.id == 0)
            info.id = 0x3FFF;
    }
    if (supervising_rbc && supervising_rbc->contact == info)
        return;
    set_rbc_contact(info);
    for (auto *session : active_sessions) {
        if (session->contact.country == info.country && session->contact.id == info.id) {
            supervising_rbc = session;
            supervising_rbc->contact.phone_number = info.phone_number;
            return;
        }
    }
    supervising_rbc = new communication_session(info, true);
    active_sessions.insert(supervising_rbc);
}
void rbc_handover(distance d, contact_info newrbc)
{
    rbc_transition_position = d;
    handover_report_accepting = handover_report_max = handover_report_min = false;
    if (supervising_rbc && supervising_rbc->contact == newrbc)
        return;
    if (accepting_rbc && accepting_rbc->contact == newrbc) {
        accepting_rbc->open(0);
        return;
    }
    if (accepting_rbc && accepting_rbc->contact != newrbc)
        accepting_rbc->close();
    if (handing_over_rbc && handing_over_rbc != supervising_rbc)
        handing_over_rbc->close();
    handing_over_rbc = supervising_rbc;
    accepting_rbc = new communication_session(newrbc, true);
    active_sessions.insert(accepting_rbc);
    accepting_rbc->open(0);
}
void terminate_session(contact_info info)
{
    if (info.id == ContactLastRBC) {
        if (rbc_contact)
            info = *rbc_contact;
        else
            return;
    }
    for (auto *session : active_sessions) {
#if SIMRAIL
        if ((session->contact.country == info.country && session->contact.id == info.id) || active_sessions.size() == 1) {
#else
        if (session->contact.country == info.country && session->contact.id == info.id) {
#endif
            session->close();
            if (session == supervising_rbc)
                supervising_rbc = nullptr;
            if (session == handing_over_rbc)
                handing_over_rbc = nullptr;
            if (session == accepting_rbc)
                accepting_rbc = handing_over_rbc = nullptr;
        }
    }
}
std::string from_bcd(uint64_t number)
{
    std::string str;
    for (int i=15; i>=0; i--)
    {
        int c = (number>>(4*i))&15;
        if (c == 15)
            continue;
        if (c >= 10)
            return "";
        str += (char)(c+48);
    }
    return str;
}
uint64_t to_bcd(std::string number)
{
    uint64_t bcd = 0;
    for (int i=0; i<16; i++) {
        if (i < number.size())
            bcd = (bcd<<4) | (number[i]-48);
        else bcd = (bcd<<4) | 0xF;
    }
    return bcd;
}
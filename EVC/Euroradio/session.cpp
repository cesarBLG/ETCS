#include "session.h"
#include "../Packets/messages.h"
#include "../Supervision/train_data.h"
#include "../Procedures/start.h"
#include "../TrainSubsystems/cold_movement.h"
#include <thread>
#include <map>
#ifndef _WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
communication_session *supervising_rbc = nullptr;
communication_session *accepting_rbc = nullptr;
communication_session *handing_over_rbc = nullptr;
safe_radio_status radio_status_driver;
std::map<contact_info, communication_session*> active_sessions;
#include <iostream>
void communication_session::open(int ntries)
{
    pending_ack.remove_if([](const msg_expecting_ack &mack){return mack.nid_ack.find(-1) != mack.nid_ack.end();});     
    if (status != session_status::Inactive)
        return;
    train_data_ack_pending = train_data_valid;
    train_data_ack_sent = false;
    closing = false;
    initsent = false;
    status = session_status::Establishing;
    radio_status = safe_radio_status::Disconnected;
    for (auto it : active_sessions) {
        if (it.second != this && it.second != accepting_rbc && it.second != handing_over_rbc && it.second != supervising_rbc)
            it.second->close();
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
    fill_message(terminate.get());
    send(terminate);
}
void communication_session::finalize()
{
    status = session_status::Inactive;
    if (radio_status == safe_radio_status::Connected)
        radio_status = safe_radio_status::Disconnected;
    if (terminal && terminal->active_session == this)
        terminal->release();
    terminal = nullptr;
    closing = false;
}
void communication_session::update()
{
    if (status == session_status::Inactive) {
        connection_timer = false;
        return;
    }
    if (terminal != nullptr)
        radio_status = terminal->status;
    if (radio_status == safe_radio_status::Connected) {
        last_active = get_milliseconds();
        connection_timer = false;
    }
    if (!isRBC && level != Level::N1 && !closing) {
        close();
    }
    if (status == session_status::Establishing) {
        if (tried == 0 || ((tried<ntries || ntries <= 0) && radio_status == safe_radio_status::Failed && (terminal == nullptr || !terminal->setting_up))) {
            if (terminal != nullptr && terminal->active_session == this) {
                terminal->active_session = nullptr;
            }
            terminal = nullptr;
            for (mobile_terminal &t : mobile_terminals) {
                if (t.setup(this)) {
                    terminal = &t;
                    tried++;
                    break;
                }
            }
        }
        if (tried >= ntries && ntries > 0 && radio_status == safe_radio_status::Failed && (terminal == nullptr || !terminal->setting_up))
            status = session_status::Inactive;
        if (radio_status == safe_radio_status::Connected && !initsent) {
            auto init = std::shared_ptr<euroradio_message_traintotrack>(new init_communication_session());
            fill_message(init.get());
            send(init);
            initsent = true;
        }
    } else if (status == session_status::Established) {
        if (radio_status == safe_radio_status::Failed && !terminal->setting_up) {
            connection_timer = true;
            if (get_milliseconds()-last_active > T_keep_session * 1000) {
                finalize();
                open(0);
            } else {
                terminal->setup(this);
            }
        }
        if (train_data_valid && train_data_ack_pending && !train_data_ack_sent) {
            train_data_ack_sent = true;
            auto *tdm = new validated_train_data_message();
            fill_message(tdm);
            auto *td = new TrainDataPacket();
            td->NC_CDTRAIN.set_value(cant_deficiency);
            td->NC_TRAIN.rawdata = 0;
            for (int t : other_train_categories) {
                td->NC_TRAIN.rawdata |= 1<<t;
            }
            td->L_TRAIN.set_value(L_TRAIN);
            td->V_MAXTRAIN.set_value(V_train);
            td->N_AXLE.rawdata = axle_number;
            td->M_AIRTIGHT.rawdata = Q_airtight ? M_AIRTIGHT_t::Fitted : M_AIRTIGHT_t::NotFitted;
            tdm->TrainData = std::shared_ptr<TrainDataPacket>(td);
            send(std::shared_ptr<validated_train_data_message>(tdm));
        }
    }
    if (terminal != nullptr) {
        std::unique_lock<std::mutex> lck(terminal->mtx);
        auto msgs = terminal->pending_read;
        terminal->pending_read.clear();
        lck.unlock();
        for (auto it = msgs.begin(); it!=msgs.end(); ++it) {
            auto msg = *it;
            if (!msg->valid || msg->readerror || (closing && msg->NID_MESSAGE != 39)) {
                continue;
            }
            int64_t timestamp = msg->T_TRAIN.get_value();
            if (timestamp < last_valid_timestamp) {
                if (msg->NID_MESSAGE != 15 && msg->NID_MESSAGE != 16)
                    continue;
            } else {
                last_valid_timestamp = timestamp;
            }
            pending_ack.remove_if([msg](const msg_expecting_ack &mack){return mack.nid_ack.find(msg->NID_MESSAGE) != mack.nid_ack.end();});
            if (msg->NID_MESSAGE == 32) {
                status = session_status::Established;
                position_report_reasons[7] = true;
                // TODO: send version
            } else if (msg->NID_MESSAGE == 39) {
                finalize();
                break;
            }
            if (msg->M_ACK == M_ACK_t::AcknowledgementRequired) {
                auto ack = std::shared_ptr<acknowledgement_message>(new acknowledgement_message());
                ack->T_TRAINack = msg->T_TRAIN;
                fill_message(ack.get());
                send(ack);
            }
            handle_radio_message(msg, this);
        }
    }
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
                if (terminal != nullptr) {
                    {
                        std::unique_lock<std::mutex> lck(terminal->mtx);
                        terminal->pending_write.push_back(msg.message);
                    }
                    terminal->cv.notify_all();
                }
            }
        }
    }
}

void communication_session::send(std::shared_ptr<euroradio_message_traintotrack> msg, bool lock)
{
    if (status == session_status::Inactive || (status == session_status::Establishing && msg->NID_MESSAGE != 155) || (closing && msg->NID_MESSAGE != 156))
        return;
    std::set<int> ack;
    if (msg->NID_MESSAGE == 129)
        ack = {8};
    else if (msg->NID_MESSAGE == 130)
        ack = {27, 28};
    else if (msg->NID_MESSAGE == 132 && ((((MA_request*)msg.get())->Q_MARQSTREASON>>Q_MARQSTREASON_t::StartSelectedByDriverBit) & 1) == 1)
        ack = {2, 3};
    else if (msg->NID_MESSAGE == 155)
        ack = {32};
    else if (msg->NID_MESSAGE == 156)
        ack = {39};
    else if (msg->NID_MESSAGE == 157)
        ack = {40,41,43};
    if (!ack.empty()) {
        pending_ack.remove_if([msg](const msg_expecting_ack &mack){return mack.message->NID_MESSAGE == msg->NID_MESSAGE;});
        pending_ack.push_back({ack,msg,1,get_milliseconds()});
    }
    if (radio_status == safe_radio_status::Connected && terminal != nullptr) {
        if (lock) {
            std::unique_lock<std::mutex> lck(terminal->mtx);
            terminal->pending_write.push_back(msg);
        } else {
            terminal->pending_write.push_back(msg);
        }
        terminal->cv.notify_all();
    }
}
std::string RadioNetworkId = "GSMR-A";
int64_t first_supervised_timestamp;
bool radio_reaction_applied = false;
bool radio_reaction_reconnected = false;
void update_euroradio()
{
    for (mobile_terminal &t : mobile_terminals) {
        if ((!t.registered || t.radio_network_id != RadioNetworkId) && (t.released == 0 && t.active_session == nullptr)) {
            t.registered = true;
            t.radio_network_id = RadioNetworkId;
        }
    }
    for (auto it = active_sessions.begin(); it != active_sessions.end(); ) {
        if (it->second->status == session_status::Inactive && it->second != supervising_rbc && it->second != accepting_rbc && it->second != handing_over_rbc) {
            delete it->second;
            it = active_sessions.erase(it);
        } else {
            it->second->update();
            ++it;
        }
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
    bool radio_hole = false;
    if (radio_hole || ((level == Level::N2 || level == Level::N3) && (!supervising_rbc || supervising_rbc->status != session_status::Established))) {
        first_supervised_timestamp = -1;
    }
    if (!radio_hole && (level == Level::N2 || level == Level::N3) && (mode == Mode::FS || mode == Mode::OS || mode == Mode::LS)) {
        if (first_supervised_timestamp < 0)
            first_supervised_timestamp = get_milliseconds();
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
        }
        if (supervising_rbc && get_milliseconds() - supervising_rbc->last_valid_timestamp < T_NVCONTACT*1000) {
            radio_reaction_applied = false;
            radio_reaction_reconnected = false;
        }
        if (get_milliseconds() - supervised_timestamp > (T_NVCONTACT + T_disconnect_radio)*1000 && !radio_reaction_reconnected) {
            radio_reaction_reconnected = true;
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                supervising_rbc->reset_radio();
        }
    } else {
        radio_reaction_applied = false;
        radio_reaction_reconnected = false;
    }
}
optional<contact_info> rbc_contact;
bool rbc_contact_valid;
void load_contact_info()
{
    //TODO: Radio Network
    rbc_contact_valid = cold_movement_status == NoColdMovement;
}
void set_supervising_rbc(contact_info info)
{
    if (info.phone_number == NID_RADIO_t::UseShortNumber) {
        info.phone_number = 5015 | ((uint64_t)inet_addr("127.0.0.1"))<<16;
    }
    if (info.id == NID_RBC_t::ContactLastRBC) {
        if (rbc_contact)
            info = *rbc_contact;
        else
            return;
    }
    if (supervising_rbc && supervising_rbc->contact == info)
        return;
    rbc_contact = info;
    if (active_sessions.find(info) != active_sessions.end()) {
        supervising_rbc = active_sessions[info];
    } else {
        supervising_rbc = new communication_session(info, true);
        active_sessions[info] = supervising_rbc;
    }
}
void terminate_session(contact_info info)
{
    if (info.id == NID_RBC_t::ContactLastRBC) {
        if (rbc_contact)
            info = *rbc_contact;
        else
            return;
    }
    for (auto &it : active_sessions) {
        if (it.first.country == info.country && it.first.id == info.id) {
            it.second->close();
            if (it.second == supervising_rbc)
                supervising_rbc = nullptr;
            if (it.second == handing_over_rbc)
                handing_over_rbc = nullptr;
            if (it.second == accepting_rbc)
                accepting_rbc = nullptr;
        }
    }
}
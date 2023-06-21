/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "terminal.h"
#include "../Time/clock.h"
#include "../optional.h"
#include "../Packets/radio.h"
#include <string>
#include <memory>
#include <set>
#include <list>
#include "platform.h"
enum struct session_status
{
    Inactive,
    Establishing,
    Established
};
extern safe_radio_status radio_status_driver;
struct msg_expecting_ack
{
    std::set<int> nid_ack;
    std::shared_ptr<euroradio_message_traintotrack> message;
    int times_sent;
    int64_t last_sent;
};
class communication_session
{
    mobile_terminal *terminal=nullptr;
    bool initsent;
    int tried;
    int ntries;
    PlatformUtil::Promise<std::shared_ptr<euroradio_message>> rx_promise;
    void message_received(std::shared_ptr<euroradio_message> msg);
    void update_ack();
    public:
    bool isRBC;
    contact_info contact;
    bool train_data_ack_pending;
    bool train_data_ack_sent;
    bool train_running_number_sent;
    bool closing;
    session_status status = session_status::Inactive;
    safe_radio_status radio_status = safe_radio_status::Disconnected;
    int64_t last_active;
    bool connection_timer;
    int64_t last_valid_timestamp = std::numeric_limits<int64_t>::lowest();
    std::list<msg_expecting_ack> pending_ack;
    int version;
    communication_session(contact_info contact, bool isRBC) : isRBC(isRBC), contact(contact), version(-1) {}
    void open(int ntries);
    void finalize();
    void close();
    void send(std::shared_ptr<euroradio_message_traintotrack> msg);
    void update();
    void reset_radio()
    {
        if (terminal != nullptr)
            terminal->status = safe_radio_status::Failed;
    }
};
extern communication_session *supervising_rbc;
extern communication_session *accepting_rbc;
extern communication_session *handing_over_rbc;
extern optional<contact_info> rbc_contact;
extern bool rbc_contact_valid;
extern bool radio_reaction_applied;
void load_contact_info();
void set_rbc_contact(contact_info contact);
void update_euroradio();
void set_supervising_rbc(contact_info info);
void terminate_session(contact_info info);
void rbc_handover(distance d, contact_info newrbc);
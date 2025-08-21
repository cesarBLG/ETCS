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
    std::shared_ptr<radio_connection> connection;
    bool initsent;
    int tried;
    int ntries;
    void message_received(std::shared_ptr<euroradio_message> msg);
    void update_ack();
    void send(std::shared_ptr<euroradio_message_traintotrack> msg);
    public:
    bool isRBC;
    contact_info contact;
    bool train_data_ack_pending;
    bool train_data_ack_sent;
    bool train_running_number_sent;
    bool accept_unknown_position=true;
    bool closing;
    session_status status = session_status::Inactive;
    safe_radio_status radio_status = safe_radio_status::Disconnected;
    int64_t last_active;
    bool connection_timer;
    int64_t last_valid_timestamp = std::numeric_limits<int64_t>::lowest();
    std::deque<std::shared_ptr<euroradio_message>> rx_list;
    std::deque<std::shared_ptr<euroradio_message_traintotrack>> tx_list;
    std::list<msg_expecting_ack> pending_ack;
    std::set<int> pending_errors;
    std::map<bg_id, std::set<bg_id>> prvlrbgs;
    int version;
    communication_session(contact_info contact, bool isRBC) : isRBC(isRBC), contact(contact), version(-1) {}
    void open(int ntries);
    void finalize();
    void close();
    void queue(std::shared_ptr<euroradio_message_traintotrack> msg);
    void send_pending();
    void update();
    void reset_radio()
    {
        if (connection != nullptr) {
            radio_status = safe_radio_status::Failed;
            connection->release();
            connection = nullptr;
            ntries = 0;
        }
    }
    void setup_connection();
    void report_error(int num);
};
const unsigned int ContactLastRBC = 16383;
const uint64_t UseShortNumber=std::numeric_limits<uint64_t>::max();
extern communication_session *supervising_rbc;
extern communication_session *accepting_rbc;
extern communication_session *handing_over_rbc;
extern distance rbc_transition_position;
extern optional<contact_info> rbc_contact;
extern bool rbc_contact_valid;
extern bool radio_reaction_applied;
extern std::set<communication_session*> active_sessions;
void load_contact_info();
void set_rbc_contact(contact_info contact);
void update_euroradio();
void set_supervising_rbc(contact_info info);
void terminate_session(contact_info info);
void rbc_handover(distance d, contact_info newrbc);
std::string from_bcd(uint64_t number);
uint64_t to_bcd(std::string number);
/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "radio_connection.h"
#include "session.h"
#include <random>
std::random_device r;
struct PDU_header
{
    unsigned char ETY:3;
    unsigned char MTI:4;
    unsigned char DF:1;
    PDU_header() {}
    PDU_header(unsigned char b)
    {
        ETY = (b>>5) & 7;
        MTI = (b>>1) & 15;
        DF = b & 1;
    }
    operator unsigned char() const
    {
        return (ETY<<5)|(MTI<<1)|DF;
    }
};

safe_radio_connection::safe_radio_connection(communication_session *session) : radio_connection(session)
{
}

void safe_radio_connection::Sa_connect_request(const called_address &address, const etcs_id &calling_address)
{
    transport = get_cfm_handle(weak_from_this());
    radio_connection::Sa_connect_request(address, calling_address);
    connection_state = radio_connection_state::WFTC;
    PDU_header header;
    header.ETY = 2;
    header.MTI = 1;
    header.DF = 0;
    std::vector<unsigned char> data;
    data.push_back(header);
    for (int i=2; i>=0; i--) {
        data.push_back((own_id.id>>(8*i))&255);
    }
    data.push_back(1);
    std::default_random_engine eng(r());
    std::uniform_int_distribution<uint64_t> uniform_dist(0, std::numeric_limits<uint64_t>::max());
    uint64_t ran = uniform_dist(eng);
    for (int i=7; i>=0; i--) {
        data.push_back((ran>>(8*i))&255);
    }
    transport->T_connect_request(peer_address, own_id, std::move(data));
}

void safe_radio_connection::Sa_data_request(std::vector<unsigned char> &&data)
{
    if (transport == nullptr)
        return;
    PDU_header header;
    header.ETY = 0;
    header.MTI = 5;
    header.DF = 0;
    data.insert(data.begin(), header);
    uint64_t mac = 0;
    for (int i=7; i>=0; i--) {
        data.push_back((mac>>(8*i))&255);
    }
    transport->T_data_request(std::move(data));
}

void safe_radio_connection::Sa_disconnect_request(int reason, int subreason)
{
    if (transport == nullptr)
        return;
    PDU_header di;
    di.ETY = 0;
    di.MTI = 8;
    di.DF = 0;
    std::vector<unsigned char> data;
    data.push_back(di);
    data.push_back(reason);
    data.push_back(subreason);
    radio_connection::Sa_disconnect_request(reason, subreason);
    connection_state = radio_connection_state::IDLE;
    transport->T_disconnect_request(std::move(data));
    transport = nullptr;
}

bool safe_radio_connection::validate_sapdu(std::vector<unsigned char> &data)
{
    PDU_header header(data[0]);
    if (header.DF != 1) {
        Sa_handle_error(6, 1);
        return false;
    }
    if (header.MTI == 2 && data.size() != 21) {
        Sa_handle_error(10, 2);
        return false;
    } else if (header.MTI == 9 && data.size() != 9) {
        Sa_handle_error(10, 4);
        return false;
    }
    if (connection_state == radio_connection_state::WFTC && header.MTI != 2) {
        Sa_handle_error(9, 1);
        return false;
    } else if (connection_state == radio_connection_state::WFAR && header.MTI != 9) {
        Sa_handle_error(9, 3);
        return false;
    }
    if (connection_state == radio_connection_state::DATA && (header.MTI == 2 || header.MTI == 9)) {
        Sa_handle_error(5, 1);
        return false;
    }
    if (header.MTI != 1 && header.MTI != 8) {
        uint64_t mac = 0;
        for (int i=0; i<8; i++) {
            mac = (mac<<8)|data[data.size() - 8 + i];
        }
        data.erase(data.begin() + data.size() - 8, data.end());
        if (false) {
            switch (header.MTI) {
                case 2:
                case 3:
                case 4:
                    Sa_handle_error(4, header.MTI);
                    break;
                default:
                    Sa_handle_error(4, 1);
                    break;
            }
            return false;
        }
    }
    return true;
}

void safe_radio_connection::T_connect_indication(etcs_id id, std::vector<unsigned char> &&data)
{
    peer_address.id = id;
    if (!validate_sapdu(data))
        return;
    PDU_header header(data[0]);
    if (header.MTI == 2) {
        PDU_header auth;
        auth.ETY = 0;
        auth.MTI = 3;
        auth.DF = 0;
        uint64_t mac=0;
        std::vector<unsigned char> au3;
        au3.push_back(auth);
        for (int i=7; i>=0; i--) {
            au3.push_back((mac>>(8*i))&255);
        }
        transport->T_data_request(std::move(au3));
        connection_state = radio_connection_state::WFAR;
    } 
}

void safe_radio_connection::T_data_indication(std::vector<unsigned char> &&data)
{
    if (!validate_sapdu(data))
        return;
    PDU_header header(data[0]);
    if (header.MTI == 9) {
        Sa_connect_confirm(peer_address.id);
        connection_state = radio_connection_state::DATA;
    } else if (header.MTI == 5) {
        data.erase(data.begin(), data.begin()+1);
        Sa_data_indication(std::move(data));
    }
}

void safe_radio_connection::Sa_handle_error(int reason, int subreason)
{
    platform->debug_print("Sa error: "+std::to_string(reason)+","+std::to_string(subreason));
    if ((reason == 8 && subreason == 1) || (reason == 8 && subreason == 5) || (reason == 10 && subreason == 1)) {
        // Reject message, do nothing
    } else if (reason == 4 && subreason == 1) {
        
    } else if ((reason == 4 && subreason == 3) || (reason == 9 && subreason == 2) || (reason == 10 && subreason == 3)) {
        if (transport == nullptr)
            return;
        PDU_header di;
        di.ETY = 0;
        di.MTI = 8;
        di.DF = 0;
        std::vector<unsigned char> data;
        data.push_back(di);
        data.push_back(reason);
        data.push_back(subreason);
        transport->T_disconnect_request(std::move(data));
        transport = nullptr;
        connection_state = radio_connection_state::IDLE;
    } else {
        Sa_disconnect_indication(reason, subreason);
        transport = nullptr;
        connection_state = radio_connection_state::IDLE;
    }
}

void safe_radio_connection::T_disconnect_indication(int reason, int subreason)
{
    Sa_handle_error(reason, subreason);
}

void safe_radio_connection::T_disconnect_indication(std::vector<unsigned char> &&data)
{
    if (validate_sapdu(data))
        Sa_handle_error(data[1], data[2]);
}
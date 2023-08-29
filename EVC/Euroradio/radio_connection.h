/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <deque>
#include <vector>
#include "../Packets/radio.h"
#include "platform_runtime.h"
class communication_session;
enum struct safe_radio_status
{
    Disconnected,
    Failed,
    Connected
};
struct etcs_id
{
    unsigned int type;
    unsigned int id;
    bool operator<(const struct etcs_id &o) const
    {
        return ((type<<24)|id) < ((o.type<<24)|o.id); 
    }
};
struct called_address
{
    etcs_id id;
    int network_id;
    uint64_t phone_number;
};

class radio_connection
{
protected:
    PlatformUtil::FulfillerBufferedQueue<std::shared_ptr<euroradio_message>> rx_list;
    PlatformUtil::Promise<void> t_estab_timer;
    etcs_id own_id;
    called_address peer_address;
    void Sa_connect_confirm(const etcs_id &id);
    void Sa_data_indication(std::vector<unsigned char>&& data);
    void Sa_disconnect_indication(int reason, int subreason);
    virtual void Sa_handle_error(int reason, int subreason) = 0;
    uint32_t bus_peer;
public:
    communication_session *session;
    safe_radio_status status = safe_radio_status::Disconnected;
    virtual void Sa_connect_request(const called_address &address, const etcs_id &calling_address);
    virtual void Sa_data_request(std::vector<unsigned char> &&msg) = 0;
    virtual void Sa_disconnect_request(int reason, int subreason);
    radio_connection(communication_session *session);
    void release();
    void send(std::shared_ptr<euroradio_message_traintotrack> msg);
    PlatformUtil::Promise<std::shared_ptr<euroradio_message>> receive();

};
class bus_radio_connection : public radio_connection
{
    std::unique_ptr<BasePlatform::BusSocket> socket;
    std::string rx_buffer;
    PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> rx_promise;
    void data_receive(BasePlatform::BusSocket::ReceiveResult &&msg);
    void Sa_handle_error(int reason, int subreason) override;
public:
    bus_radio_connection(communication_session *session);
    void Sa_connect_request(const called_address &address, const etcs_id &calling_address) override;
    void Sa_data_request(std::vector<unsigned char> &&msg) override;
    void Sa_disconnect_request(int reason, int subreason) override;
};
class cfm;
class safe_radio_connection : public radio_connection, public std::enable_shared_from_this<safe_radio_connection>
{
    enum struct radio_connection_state
    {
        WFTC,
        WFAR,
        DATA,
        WFRESP,
        IDLE
    };
    std::shared_ptr<cfm> transport;
    radio_connection_state connection_state = radio_connection_state::IDLE;
    void Sa_handle_error(int reason, int subreason) override;
    bool validate_sapdu(std::vector<unsigned char> &data);
public:
    safe_radio_connection(communication_session *session);
    void Sa_connect_request(const called_address &address, const etcs_id &calling_address) override;
    void Sa_data_request(std::vector<unsigned char> &&data) override;
    void Sa_disconnect_request(int reason, int subreason) override;

    void T_connect_indication(etcs_id id, std::vector<unsigned char> &&data);
    void T_data_indication(std::vector<unsigned char> &&sapdu);
    void T_disconnect_indication(int reason, int subreason);
    void T_disconnect_indication(std::vector<unsigned char> &&data);
};

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
#include "../Version/translate.h"
void radio_connection::Sa_connect_confirm(const etcs_id &id)
{
    t_estab_timer = {};
    status = safe_radio_status::Connected;
    peer_address.id = id;
    if (session != nullptr) session->contact.id = id.id;
}
void radio_connection::Sa_data_indication(std::vector<unsigned char>&& data)
{
    if (session == nullptr)
        return;
    bit_manipulator r(std::move(data));
    rx_list.push_data(euroradio_message::build(r, session->version));
}
void radio_connection::Sa_disconnect_indication(int reason, int subreason)
{
    t_estab_timer = {};
    status = reason == 0 ? safe_radio_status::Disconnected : safe_radio_status::Failed;
}
void radio_connection::Sa_connect_request(const called_address &address, const etcs_id &calling_address)
{
    own_id = calling_address;
    peer_address = address;
    t_estab_timer = platform->delay(40000);
    t_estab_timer.then([this]() {
        if (status == safe_radio_status::Disconnected) {
            Sa_handle_error(7, 3);
        }
    });
}
void radio_connection::Sa_disconnect_request(int reason, int subreason)
{
    t_estab_timer = {};
    status = reason == 0 ? safe_radio_status::Disconnected : safe_radio_status::Failed;
}
radio_connection::radio_connection(communication_session *session) : session(session)
{
}
void radio_connection::release()
{
    Sa_disconnect_request(0, 0);
}
void radio_connection::send(std::shared_ptr<euroradio_message_traintotrack> msg)
{
    if (session == nullptr)
        return;
    msg = translate_message(msg, session->version);
    bit_manipulator w;
    msg->write_to(w);
    Sa_data_request(std::move(w.bits));
}
PlatformUtil::Promise<std::shared_ptr<euroradio_message>> radio_connection::receive() {
    return rx_list.create_and_add();
}

bus_radio_connection::bus_radio_connection(communication_session *session) : radio_connection(session)
{
}
void bus_radio_connection::Sa_connect_request(const called_address &address, const etcs_id &calling_address)
{
    radio_connection::Sa_connect_request(address, calling_address);
    std::string rbc = "rbc_";
    if (peer_address.phone_number == 0xFFFFFFFFFFFFFFFFULL)
        rbc += "5015";
    else
        rbc += from_bcd(peer_address.phone_number);
    socket = platform->open_socket(rbc, BasePlatform::BusSocket::PeerId::fourcc("EVC"));
    if (socket) {
        status = safe_radio_status::Connected;
        rx_promise = socket->receive().then(std::bind(&bus_radio_connection::data_receive, this, std::placeholders::_1));
    } else {
        Sa_handle_error(1, 1);
    }
}
void bus_radio_connection::data_receive(BasePlatform::BusSocket::ReceiveResult &&result)
{
    rx_promise = socket->receive().then(std::bind(&bus_radio_connection::data_receive, this, std::placeholders::_1));

    if (!std::holds_alternative<BasePlatform::BusSocket::Message>(result))
        return;
    auto data = std::move(std::get<BasePlatform::BusSocket::Message>(result).data);

    if (rx_buffer.empty())
        rx_buffer = std::move(data);
    else
        rx_buffer += std::move(data);

    while (true) {
        if (rx_buffer.size() < 3)
            return;

        size_t size = ((rx_buffer[1]&255)<<2)|((rx_buffer[2]&255)>>6);

        if (rx_buffer.size() < size)
            return;

        std::vector<unsigned char> pack;
        pack.insert(pack.end(), rx_buffer.begin(), rx_buffer.begin() + size);
        rx_buffer.erase(0, size);
        radio_connection::Sa_data_indication(std::move(pack));
    }
}
void bus_radio_connection::Sa_data_request(std::vector<unsigned char> &&data)
{
    if (socket)
        socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("RBC"), std::string((char*)data.data(), data.size()));
}
void bus_radio_connection::Sa_disconnect_request(int reason, int subreason)
{
    radio_connection::Sa_disconnect_request(reason, subreason);
    socket = nullptr;
    rx_promise = {};
}
void bus_radio_connection::Sa_handle_error(int reason, int subr)
{
    Sa_disconnect_indication(reason, subr);
    socket = nullptr;
    rx_promise = {};
}
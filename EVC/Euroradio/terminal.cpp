/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "terminal.h"
#include "session.h"
#include "../Version/translate.h"
#include "platform_runtime.h"
#include "platform_terminals.h"
mobile_terminal *mobile_terminals[2] = {new bus_mobile_terminal(), new bus_mobile_terminal()};

optional<std::vector<std::string>> AllowedRadioNetworks;
std::string RadioNetworkId = "GSMR-A";

void mobile_terminal::data_receive(std::string &&data)
{
    if (rx_buffer.empty())
        rx_buffer = std::move(data);
    else
        rx_buffer += std::move(data);

    while (true) {
        if (rx_buffer.size() < 3)
            return;

        size_t size = ((unsigned char)rx_buffer[1]<<2)|((unsigned char)rx_buffer[2]>>6);

        if (rx_buffer.size() < size)
            return;

        std::vector<unsigned char> pack;
        pack.insert(pack.end(), rx_buffer.begin(), rx_buffer.begin() + size);
        rx_buffer.erase(0, size);

        bit_manipulator r(std::move(pack));
        rx_list.push_data(euroradio_message::build(r, active_session == nullptr ? -1 : active_session->version));
    }
}

void mobile_terminal::send(std::shared_ptr<euroradio_message_traintotrack> msg)
{
    msg = translate_message(msg, 0);
    bit_manipulator w;
    msg->write_to(w);
    send(w.bits.data(), w.bits.size());
}

void mobile_terminal::release()
{
    rx_buffer.clear();
    active_session = nullptr;
    status = safe_radio_status::Disconnected;
}

void mobile_terminal::update()
{
    if (!registered && radio_network_id != "" && last_register_order && get_milliseconds() - *last_register_order > 4000)
        registered = true;
    if (registered && radio_network_id == "")
        registered = false;
    if (registered && last_register_order)
        last_register_order = {};
}

PlatformUtil::Promise<std::shared_ptr<euroradio_message>> mobile_terminal::receive() {
    return rx_list.create_and_add();
}

void bus_mobile_terminal::data_receive(BasePlatform::BusSocket::ReceiveResult &&result)
{
    rx_promise = socket->receive().then(std::bind(&bus_mobile_terminal::data_receive, this, std::placeholders::_1));

    if (std::holds_alternative<BasePlatform::BusSocket::JoinNotification>(result)) {
        status = safe_radio_status::Connected;
        return;
    }
    if (std::holds_alternative<BasePlatform::BusSocket::LeaveNotification>(result)) {
        if (status == safe_radio_status::Connected)
            status = safe_radio_status::Failed;
        rx_promise = {};
        rx_buffer.clear();
        socket = nullptr;
        released = 0;
        return;
    }
    auto msg = std::move(std::get<BasePlatform::BusSocket::Message>(result));
    mobile_terminal::data_receive(std::move(msg.data));
}

void bus_mobile_terminal::send(unsigned char *data, size_t size)
{
    if (socket)
        socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("RBC"), std::string((char*)data, size));
}

bool bus_mobile_terminal::setup(communication_session *session)
{
    if (released > 0 || !registered)
        return false;

    socket = platform->open_socket("rbc_" + std::to_string(session->contact.phone_number), BasePlatform::BusSocket::PeerId::fourcc("EVC"));
    if (socket) {
        active_session = session;
        released = 2;

        rx_promise = socket->receive().then(std::bind(&bus_mobile_terminal::data_receive, this, std::placeholders::_1));
    } else {
        status = safe_radio_status::Failed;
    }

    return true;
}

void bus_mobile_terminal::release()
{
    socket = nullptr;
    released = 0;

    mobile_terminal::release();
}
/*
void internet_mobile_terminal::update()
{
    mobile_terminal::update();
}

bool internet_mobile_terminal::setup(communication_session *session)
{
    if (released > 0 || !registered)
        return false;
    std::string hostname = "";
    for (int i=3; i>=0; i--)
    {
        hostname += std::to_string((session->contact.phone_number>>(i*8)) & 255);
        if (i>0) hostname += ".";
    }
    int port = session->contact.phone_number & 65535;
    socket = std::make_unique<TcpSocket>(hostname, port, FdPoller());
    if (socket) {
        active_session = session;
        released = 2;
        socket->receive().then(std::bind(&internet_mobile_terminal::data_receive, this, std::placeholders::_1));
        return true;
    }
    return false;
}

void internet_mobile_terminal::release()
{
    socket = nullptr;
    released = 0;
    rx_promise = {};
    mobile_terminal::release();
}

void internet_mobile_terminal::send(unsigned char *data, size_t size)
{
    socket->send(std::string((char*)data, size));
}

void internet_mobile_terminal::data_receive(std::string &&data)
{
    rx_promise = socket->receive().then(std::bind(&internet_mobile_terminal::data_receive, this, std::placeholders::_1));
    mobile_terminal::data_receive(std::move(data));
}

void internet_mobile_terminal::update()
{
    mobile_terminal::update();
    if (status == safe_radio_status::Connected && (!socket || !socket->is_connected())) {
        socket = nullptr;
        released = 0;
        rx_promise = {};
        status = safe_radio_status::Failed;
    }
    if (status == safe_radio_status::Disconnected && socket && socket->is_connected()) {
        status = safe_radio_status::Connected;
    }
}*/

void retrieve_radio_networks()
{
    AllowedRadioNetworks = {"GSMR-A", "GSMR-B"};
}

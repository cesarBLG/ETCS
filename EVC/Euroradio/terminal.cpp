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

mobile_terminal mobile_terminals[2];

void mobile_terminal::data_received(std::pair<BasePlatform::BusSocket::ClientId, std::string> &&data) {
    rx_promise = std::move(socket->receive().then(std::bind(&mobile_terminal::data_received, this, std::placeholders::_1)));

    std::string buffer = std::move(data.second);
    if (buffer.size() < 3)
        return;

    int size = (buffer[1]<<2)|(buffer[2]>>6);
    if (buffer.size() < size)
        return;

    std::vector<unsigned char> pack;
    pack.insert(pack.end(), buffer.begin(), buffer.end());

    bit_manipulator r(std::move(pack));
    rx_list.fulfill_one(euroradio_message::build(r, active_session == nullptr ? -1 : active_session->version));
}

void mobile_terminal::send(std::shared_ptr<euroradio_message_traintotrack> msg) {
    msg = translate_message(msg, 0);
    bit_manipulator w;
    msg->write_to(w);
    if (socket)
        socket->broadcast(BasePlatform::BusSocket::ClientId::fourcc("RBC_"), std::string((char*)w.bits.data(), w.bits.size()));
}

PlatformUtil::Promise<std::shared_ptr<euroradio_message>> mobile_terminal::receive() {
    return rx_list.create_and_add();
}

bool mobile_terminal::setup(communication_session *session)
{
    if (released > 0 || !registered)
        return false;

    socket = platform->open_socket("rbc_" + std::to_string(session->contact.phone_number), BasePlatform::BusSocket::ClientId::fourcc("EVC_"));
    if (socket) {
        active_session = session;
        status = safe_radio_status::Connected;
        released = 2;

        rx_promise = std::move(socket->receive().then(std::bind(&mobile_terminal::data_received, this, std::placeholders::_1)));
    } else {
        status = safe_radio_status::Failed;
    }

    return true;
}
void mobile_terminal::release()
{
    rx_promise = {};
    socket = nullptr;

    active_session = nullptr;
    status = safe_radio_status::Disconnected;
    released = 0;
}

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
#include "safe_radio.h"
#include "platform_runtime.h"
class communication_session;
class mobile_terminal;
enum struct safe_radio_status
{
    Disconnected,
    Failed,
    Connected
};
class safe_radio_connection
{
protected:
    std::string rx_buffer;
    PlatformUtil::FulfillerBufferedQueue<std::shared_ptr<euroradio_message>> rx_list;
    int64_t connect_time;
    void data_receive(std::string &&data);
    virtual void send(unsigned char *data, size_t size) = 0;
public:
    communication_session *active_session;
    mobile_terminal *terminal;
    safe_radio_status status = safe_radio_status::Disconnected;
    bool setting_up = false;
    safe_radio_connection(communication_session *session, mobile_terminal *terminal);
    virtual ~safe_radio_connection();
    virtual void update();
    virtual void release() = 0;
    void send(std::shared_ptr<euroradio_message_traintotrack> msg);
    PlatformUtil::Promise<std::shared_ptr<euroradio_message>> receive();
};
class bus_safe_connection : public safe_radio_connection
{
    std::unique_ptr<BasePlatform::BusSocket> socket;
    PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> rx_promise;
    void data_receive(BasePlatform::BusSocket::ReceiveResult &&msg);
public:
    bus_safe_connection(communication_session *session, mobile_terminal *terminal);
    void update() override;
    void release() override;
    void send(unsigned char *data, size_t size) override;
};
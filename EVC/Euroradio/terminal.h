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
#include "../Packets/radio.h"
#include "platform.h"
class communication_session;
struct contact_info
{
    unsigned int country;
    unsigned int id;
    uint64_t phone_number;
    bool operator<(const contact_info &o) const
    {
        if (country == o.country) {
            if (id == o.id)
                return phone_number<o.phone_number;
            return id < o.id;
        }
        return country < o.country;
    }
    bool operator==(const contact_info &o) const
    {
        return country == o.country && id == o.id && phone_number == o.phone_number;
    }
    bool operator!=(const contact_info &o) const
    {
        return !(*this == o);
    }
};
enum struct safe_radio_status
{
    Disconnected,
    Failed,
    Connected
};
class mobile_terminal
{
    std::unique_ptr<BasePlatform::BusSocket> socket;
    std::string rx_buffer;

    PlatformUtil::Promise<std::pair<BasePlatform::BusSocket::PeerId, std::string>> rx_promise;
    PlatformUtil::FulfillerList<std::shared_ptr<euroradio_message>> rx_list;
    void data_received(std::pair<BasePlatform::BusSocket::PeerId, std::string> &&data);

public:
    communication_session *active_session = nullptr;
    bool setting_up = false;
    int released = 0;

    safe_radio_status status = safe_radio_status::Disconnected;
    std::string radio_network_id;
    bool registered;
    bool setup(communication_session *session);
    void release();

    void send(std::shared_ptr<euroradio_message_traintotrack> msg);
    PlatformUtil::Promise<std::shared_ptr<euroradio_message>> receive();
};
extern mobile_terminal mobile_terminals[2];
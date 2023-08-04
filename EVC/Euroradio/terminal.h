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
#include "platform.h"
#include <functional>
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
class mobile_terminal
{
public:
    std::set<std::shared_ptr<safe_radio_connection>> connections;
    std::string radio_network_id;
    optional<int64_t> last_register_order;
    bool registered;
    mobile_terminal();
    void update();
    std::function<std::shared_ptr<safe_radio_connection>(communication_session *session)> setup_connection;
};
extern mobile_terminal *mobile_terminals[2];
extern optional<std::vector<std::string>> AllowedRadioNetworks;
extern std::string RadioNetworkId;
void retrieve_radio_networks();
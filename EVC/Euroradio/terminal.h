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
#include "radio_connection.h"
#include "cfm.h"
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
    bool powered;
    std::list<std::weak_ptr<cfm>> ps_connections;
    std::optional<std::weak_ptr<cfm>> cs_connection;
    bool csd_available;
    bool mobile_data_available;
    int network_id;
    optional<int64_t> last_register_order;
    bool registered;
    mobile_terminal();
    void update();
    void network_register(int network);
    void set_mobile_data(bool active);
};
extern std::vector<mobile_terminal*> mobile_terminals;
extern std::map<int, std::string> RadioNetworkNames;
extern optional<std::vector<int>> AllowedRadioNetworks;
extern int RadioNetworkId;
void retrieve_radio_networks();
std::string radio_network_name(int id);
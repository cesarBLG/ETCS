/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "cfm.h"
#include <set>
#include <memory>
#include "terminal.h"
std::set<std::shared_ptr<cfm>> cfm_connections;

void cfm::T_connect_indication(etcs_id id, std::vector<unsigned char> &&data)
{
    if (auto conn = connection.lock())
        conn->T_connect_indication(id, std::move(data));
    else
        state = cfm_state::Released;
}
void cfm::T_data_indication(std::vector<unsigned char> &&data)
{
    if (auto conn = connection.lock())
        conn->T_data_indication(std::move(data));
    else
        state = cfm_state::Released;
}
void cfm::T_disconnect_indication(std::vector<unsigned char> &&data)
{
    if (auto conn = connection.lock())
        conn->T_disconnect_indication(std::move(data));
    else
        state = cfm_state::Released;
}
void cfm::T_disconnect_indication(int r, int subr)
{
    if (auto conn = connection.lock())
        conn->T_disconnect_indication(r, subr);
    else
        state = cfm_state::Released;
}

void update_cfm()
{
    for (mobile_terminal *t : mobile_terminals) {
        t->update();
    }
    for (auto it = cfm_connections.begin(); it != cfm_connections.end();) {
        (*it)->update();
        if ((*it)->state == cfm::cfm_state::Released)
            it = cfm_connections.erase(it);
        else
            ++it;
    }
}
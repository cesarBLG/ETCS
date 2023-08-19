/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <vector>
#include <functional>
#include <optional>
#include "radio_connection.h"

class cfm : public std::enable_shared_from_this<cfm>
{
protected:
    void T_connect_indication(etcs_id id, std::vector<unsigned char> &&data);
    void T_data_indication(std::vector<unsigned char> &&data);
    void T_disconnect_indication(int reason, int subreason);
    void T_disconnect_indication(std::vector<unsigned char> &&data);
    virtual void handle_error(int reason, int subreason)
    {
        T_disconnect_indication(1, reason);
    }
    std::weak_ptr<safe_radio_connection> connection;
public:
    enum struct cfm_state
    {
        Initial,
        Connecting,
        Connected,
        Releasing,
        Released,
    } state;
    cfm(std::weak_ptr<safe_radio_connection> conn) : connection(conn)
    {
        state = cfm_state::Initial;
    }
    virtual ~cfm() {}
	virtual void T_connect_request(called_address address, etcs_id calling_address, std::vector<unsigned char> &&data) = 0;
    virtual void T_data_request(std::vector<unsigned char> &&ale) = 0;
    virtual void T_disconnect_request(std::vector<unsigned char> &&data) = 0;
    virtual void update() {}
};

extern std::set<std::shared_ptr<cfm>> cfm_connections;

std::shared_ptr<cfm> get_cfm_handle(std::weak_ptr<safe_radio_connection> conn);
void update_cfm();
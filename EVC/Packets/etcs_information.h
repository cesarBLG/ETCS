/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <list>
#include <memory>
#include <functional>
#include "../Position/distance.h"
#include "../Position/linking.h"
#include "messages.h"
struct etcs_information
{
    int index_level;
    int index_mode;
    std::function<void()> handle_fun;
    virtual void handle() {handle_fun();}
    distance ref;
    int64_t timestamp;
    optional<bg_id> infill;
    communication_session *fromRBC;
    int dir;
    bg_id nid_bg;
    int version;
    bool reevaluated = false;
    optional<std::shared_ptr<euroradio_message>> message;
    std::list<std::shared_ptr<ETCS_packet>> linked_packets;
    etcs_information() : index_mode(-1), index_level(-1){}
    etcs_information(int index) : index_mode(index), index_level(index){}
    etcs_information(int index_level, int index_mode, std::function<void()> fun = nullptr) : index_level(index_level), index_mode(index_mode), handle_fun(fun) {}
};
void try_handle_information(std::shared_ptr<etcs_information> info, std::list<std::shared_ptr<etcs_information>> message);
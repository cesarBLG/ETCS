/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "logging.h"
#include "platform_runtime.h"

std::unique_ptr<BasePlatform::BusSocket> logging_socket;
void start_logging()
{
    logging_socket = platform->open_socket("evc_logging", BasePlatform::BusSocket::PeerId::fourcc("EVC"));
}
void print_vars(std::string &str, std::vector<std::pair<std::string,std::string>> vars)
{
    for (auto &var : vars)
    {
        str += var.first + '\t' + var.second + '\n';
    }
}
void log_message(ETCS_message &msg, dist_base &dist, int64_t time)
{
    if (!logging_socket)
        return;
    bit_manipulator b;
    msg.write_to(b);
    std::string str = "Distance: " + std::to_string(dist.dist+odometer_reference) + "\t Time: " + std::to_string(time) + "\n";
    print_vars(str, b.log_entries);
#ifdef DEBUG_TRACK_MESSAGES
    platform->debug_print(str);
#endif
    logging_socket->broadcast(str);
}

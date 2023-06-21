/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "logging.h"
#include "platform.h"

std::unique_ptr<BasePlatform::BusSocket> logging_socket;
void start_logging()
{
    logging_socket = platform->open_socket("evc_logging", BasePlatform::BusSocket::ClientId::fourcc("EVC"));
}
void print_vars(std::string &str, std::vector<std::pair<std::string,std::string>> vars)
{
    for (auto &var : vars)
    {
        str += var.first + '\t' + var.second + '\n';
    }
}
void log_message(std::shared_ptr<ETCS_message> msg, distance &dist, int64_t time)
{
    if (!logging_socket)
        return;
    bit_manipulator b;
    msg->write_to(b);
    std::string str = "Distance: " + std::to_string(dist.get()+odometer_reference) + "\t Time: " + std::to_string(time) + "\n";
    print_vars(str, b.log_entries);
    logging_socket->broadcast(str);
}

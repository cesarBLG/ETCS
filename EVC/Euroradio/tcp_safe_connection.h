/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "safe_radio.h"
#include "tcp_socket.h"
#include "dns.h"
class tcp_safe_connection : public safe_radio_connection
{
    std::unique_ptr<DNSQuery> dns_query;
    std::unique_ptr<TcpSocket> socket;
    PlatformUtil::Promise<std::string> rx_promise;
    FdPoller &poller;
    void data_receive(std::string &&msg);
    void connect(dns_entry &&e);
public:
    tcp_safe_connection(communication_session *session, mobile_terminal *terminal, FdPoller &poller);
    void update() override;
    void release() override;
    void send(unsigned char *data, size_t size) override;
};
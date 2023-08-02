/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "terminal.h"
#include "tcp_socket.h"
class bus_mobile_terminal : public mobile_terminal
{
    std::unique_ptr<BasePlatform::BusSocket> socket;
    PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> rx_promise;
    void data_receive(BasePlatform::BusSocket::ReceiveResult &&msg);

public:
    bool setup(communication_session *session) override;
    void release() override;
    void send(unsigned char *data, size_t size) override;
};
/*class internet_mobile_terminal : public mobile_terminal
{
    std::unique_ptr<TcpSocket> socket;
    PlatformUtil::Promise<std::string> rx_promise;
    void data_receive(std::string &&msg);
public:
    bool setup(communication_session *session) override;
    void release() override;
    void send(unsigned char *data, size_t size) override;
    void update() override;
};*/
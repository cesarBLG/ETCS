/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "platform_runtime.h"
#include "tcp_socket.h"
#include "bus_socket_impl.h"
class OrtsBridge
{
private:
    FdPoller& poller;
    BusSocketImpl& bus_impl;

    std::unique_ptr<BasePlatform::BusSocket> bus_socket;
    PlatformUtil::Promise<BasePlatform::BusSocket::ReceiveResult> bus_rx_promise;

    std::unique_ptr<TcpSocket> tcp_socket;
    PlatformUtil::Promise<std::string> tcp_rx_promise;
    std::string tcp_rx_buffer;
    void tcp_rx(std::string &&data);
    void bus_rx(BasePlatform::BusSocket::ReceiveResult &&result);
    void setup();
public:
    OrtsBridge(const std::string_view load_path, FdPoller &fd, BusSocketImpl &impl);
};
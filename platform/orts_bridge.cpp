/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "orts_bridge.h"
#include "console_platform.h"
#include <orts/ip_discovery.h>
#include <fstream>
OrtsBridge::OrtsBridge(const std::string_view load_path, FdPoller &fd, BusSocketImpl &impl) : poller(fd), bus_impl(impl)
{
    std::ifstream file(std::string(load_path) + "orts_bridge.conf", std::ios::binary);
    bool start=false;
    file>>start;
    if (start)
        setup();
}

void OrtsBridge::tcp_rx(std::string &&data)
{
    if (data.empty()) {
        tcp_rx_buffer.clear();
        platform->delay(1000).then(std::bind(&OrtsBridge::setup, this)).detach();
        return;
    }

    tcp_rx_promise = tcp_socket->receive().then(std::bind(&OrtsBridge::tcp_rx, this, std::placeholders::_1));

    if (tcp_rx_buffer.empty())
        tcp_rx_buffer = std::move(data);
    else
        tcp_rx_buffer += std::move(data);

    size_t it;
    while ((it = tcp_rx_buffer.find_first_of("\r\n")) != -1) {
        if (it != 0)
            bus_socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("EVC"), tcp_rx_buffer.substr(0, it));
        tcp_rx_buffer.erase(0, tcp_rx_buffer.find_first_not_of("\r\n", it));
    }
}

void OrtsBridge::bus_rx(BasePlatform::BusSocket::ReceiveResult &&result)
{
    bus_rx_promise = bus_socket->receive().then(std::bind(&OrtsBridge::bus_rx, this, std::placeholders::_1));

    if (!std::holds_alternative<BasePlatform::BusSocket::Message>(result))
        return;
    auto msg = std::move(std::get<BasePlatform::BusSocket::Message>(result));

    tcp_socket->send(msg.data + "\r\n");
}

void OrtsBridge::setup()
{
    tcp_socket = nullptr;

    bus_socket = bus_impl.open_bus_socket("evc_sim", BasePlatform::BusSocket::PeerId::fourcc("SRV"));
    if (!bus_socket)
        return;
    bus_rx_promise = bus_socket->receive().then(std::bind(&OrtsBridge::bus_rx, this, std::placeholders::_1));

    std::string ip = ORserver::discover_server_ip();
    if (ip == "") ip = "127.0.0.1";
    tcp_socket = std::make_unique<TcpSocket>(ip, 5090, poller);
    tcp_rx_promise = tcp_socket->receive().then(std::bind(&OrtsBridge::tcp_rx, this, std::placeholders::_1));
}
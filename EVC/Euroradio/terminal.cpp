/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "terminal.h"
#include "session.h"
#include "../Version/translate.h"
#include "platform_runtime.h"
#include "platform_terminals.h"
mobile_terminal *mobile_terminals[2] = {new mobile_terminal(), new mobile_terminal()};

optional<std::vector<std::string>> AllowedRadioNetworks;
std::string RadioNetworkId = "GSMR-A";

safe_radio_connection::safe_radio_connection(communication_session *session, mobile_terminal *terminal) : active_session(session), terminal(terminal)
{
    setting_up = true;
    connect_time = get_milliseconds();
}

safe_radio_connection::~safe_radio_connection()
{
}

void safe_radio_connection::data_receive(std::string &&data)
{
    if (rx_buffer.empty())
        rx_buffer = std::move(data);
    else
        rx_buffer += std::move(data);

    while (true) {
        if (rx_buffer.size() < 3)
            return;

        size_t size = ((unsigned char)rx_buffer[1]<<2)|((unsigned char)rx_buffer[2]>>6);

        if (rx_buffer.size() < size)
            return;

        std::vector<unsigned char> pack;
        pack.insert(pack.end(), rx_buffer.begin(), rx_buffer.begin() + size);
        rx_buffer.erase(0, size);

        bit_manipulator r(std::move(pack));
        rx_list.push_data(euroradio_message::build(r, active_session == nullptr ? -1 : active_session->version));
    }
}

void safe_radio_connection::send(std::shared_ptr<euroradio_message_traintotrack> msg)
{
    msg = translate_message(msg, 0);
    bit_manipulator w;
    msg->write_to(w);
    send(w.bits.data(), w.bits.size());
}

PlatformUtil::Promise<std::shared_ptr<euroradio_message>> safe_radio_connection::receive() {
    return rx_list.create_and_add();
}

void safe_radio_connection::update()
{
    
}

bus_safe_connection::bus_safe_connection(communication_session *session, mobile_terminal *terminal) : safe_radio_connection(session, terminal)
{
    socket = platform->open_socket("rbc_" + std::to_string(session->contact.phone_number), BasePlatform::BusSocket::PeerId::fourcc("EVC"));
    if (socket) {
        rx_promise = socket->receive().then(std::bind(&bus_safe_connection::data_receive, this, std::placeholders::_1));
    } else {
        status = safe_radio_status::Failed;
    }
}

void bus_safe_connection::data_receive(BasePlatform::BusSocket::ReceiveResult &&result)
{
    rx_promise = socket->receive().then(std::bind(&bus_safe_connection::data_receive, this, std::placeholders::_1));

    if (std::holds_alternative<BasePlatform::BusSocket::JoinNotification>(result)) {
        status = safe_radio_status::Connected;
        setting_up = false;
        return;
    }
    if (std::holds_alternative<BasePlatform::BusSocket::LeaveNotification>(result)) {
        if (status == safe_radio_status::Connected)
            status = safe_radio_status::Failed;
        rx_promise = {};
        rx_buffer.clear();
        socket = nullptr;
        return;
    }
    auto msg = std::move(std::get<BasePlatform::BusSocket::Message>(result));
    safe_radio_connection::data_receive(std::move(msg.data));
}

void bus_safe_connection::update()
{
    if (setting_up && get_milliseconds() - connect_time > 40000) {
        socket = nullptr;
        rx_promise = {};
        status = safe_radio_status::Failed;
    }
}

void bus_safe_connection::send(unsigned char *data, size_t size)
{
    if (socket)
        socket->broadcast(BasePlatform::BusSocket::PeerId::fourcc("RBC"), std::string((char*)data, size));
}

void bus_safe_connection::release()
{
    socket = nullptr;
    rx_promise = {};
    status = safe_radio_status::Disconnected;
}
#ifdef _WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>
#include <iomanip>
#endif
tcp_safe_connection::tcp_safe_connection(communication_session *session, mobile_terminal *terminal, FdPoller &poller) : safe_radio_connection(session, terminal)
{
    setting_up = true;
    std::string hostname = "";
    for (int i=3; i>=0; i--)
    {
        hostname += std::to_string((session->contact.phone_number>>(i*8+16)) & 255);
        if (i>0) hostname += ".";
    }
    int port = session->contact.phone_number & 65535;
    /*std::string hostname = "id";
    std::stringstream ss;
    ss << std::setfill ('0') << std::setw(6) << std::hex << ((session->contact.country<<14) | session->contact.id);
    hostname += ss.str();
    if (session->isRBC)
        hostname += ".ty01";
    else
        hostname += ".ty00";
    hostname += ".etcs";
    int port = 30998;*/
    platform->debug_print("Connecting to RBC at "+hostname+":"+std::to_string(port));
    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        status = safe_radio_status::Failed;
        return;
    }
    int t;
    t = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &t, sizeof(t));
    t = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &t, sizeof(t));
    t = 12;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &t, sizeof(t));
    t = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &t, sizeof(t));
    t = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &t, sizeof(t));
    #ifndef _WIN32
    t = 20000;
    setsockopt(sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &t, sizeof(t));
    t = 1416;
    setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &t, sizeof(t));
    #endif
    socket = std::make_unique<TcpSocket>(sock, poller);
    socket->connect(hostname, port);
    rx_promise = socket->receive().then(std::bind(&tcp_safe_connection::data_receive, this, std::placeholders::_1));
}

void tcp_safe_connection::send(unsigned char *data, size_t size)
{
    socket->send(std::string((char*)data, size));
}

void tcp_safe_connection::data_receive(std::string &&data)
{
    rx_promise = socket->receive().then(std::bind(&tcp_safe_connection::data_receive, this, std::placeholders::_1));
    safe_radio_connection::data_receive(std::move(data));
}

void tcp_safe_connection::release()
{
    socket = nullptr;
    rx_promise = {};
    status = safe_radio_status::Disconnected;
}

void tcp_safe_connection::update()
{
    safe_radio_connection::update();
    if (!socket || (setting_up && get_milliseconds() - connect_time > 40000) || socket->is_failed()) {
        socket = nullptr;
        rx_promise = {};
        status = safe_radio_status::Failed;
    }
    if (status == safe_radio_status::Disconnected && socket && socket->is_connected()) {
        status = safe_radio_status::Connected;
        setting_up = false;
    }
}

mobile_terminal::mobile_terminal()
{
    setup_connection = [this](communication_session *session) {
        if (!registered || radio_network_id != RadioNetworkId)
            return std::shared_ptr<safe_radio_connection>(nullptr);
        auto conn = std::shared_ptr<safe_radio_connection>(new bus_safe_connection(session, this));
        connections.insert(conn);
        return conn;
    };
}

void mobile_terminal::update()
{
    if (!registered && radio_network_id != "" && last_register_order && get_milliseconds() - *last_register_order > 4000)
        registered = true;
    if (registered && radio_network_id == "")
        registered = false;
    if (registered && last_register_order)
        last_register_order = {};
    for (auto it=connections.begin(); it != connections.end();) {
        auto conn = *it;
        conn->update();
        if (conn->status == safe_radio_status::Failed || (conn->status == safe_radio_status::Disconnected && !conn->setting_up))
            it = connections.erase(it);
        else
            ++it;
    }
}

void retrieve_radio_networks()
{
    AllowedRadioNetworks = {"GSMR-A", "GSMR-B"};
}

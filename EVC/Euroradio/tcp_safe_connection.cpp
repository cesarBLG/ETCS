/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "tcp_safe_connection.h"
#include "session.h"
#include <thread>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif
tcp_safe_connection::tcp_safe_connection(communication_session *session, mobile_terminal *terminal, FdPoller &poller) : safe_radio_connection(session, terminal), poller(poller)
{
    std::string domain = "etcs.vtrains.dedyn.io";
    std::string hostname = "id";
    std::stringstream ss;
    ss << std::setfill ('0') << std::setw(6) << std::hex << ((session->contact.country<<14) | session->contact.id);
    hostname += ss.str();
    if (session->isRBC)
        hostname += ".ty01";
    else
        hostname += ".ty00";
    hostname += ".";
    hostname += domain;
    platform->debug_print("Trying RBC "+hostname);

    std::thread thr([hostname, this]() {
        addrinfo hints = {}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo(hostname.c_str(), "30998", &hints, &res);
        std::unique_lock<std::mutex> lck(mtx);
        if (res == nullptr) {
            dns_query = "failed";
            return;
        }
        char buff[INET_ADDRSTRLEN];
        const char *ip = inet_ntop(AF_INET, &((const sockaddr_in *)res->ai_addr)->sin_addr, buff, INET_ADDRSTRLEN);
        if (ip == nullptr)
            dns_query = "failed";
        else
            dns_query = std::string(ip);
	    freeaddrinfo(res);
    });
    thr.detach();
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
    std::unique_lock<std::mutex> lck(mtx);
    if (setting_up && socket == nullptr && (dns_query != "" || get_milliseconds() - connect_time > 5000)) {
        std::string ip;
        int port;
        if (dns_query != "" && dns_query != "failed") {
            ip = dns_query;
            port = 30998;
        } else {
            for (int i=3; i>=0; i--)
            {
                ip += std::to_string((active_session->contact.phone_number>>(i*8+16)) & 255);
                if (i>0) ip += ".";
            }
            port = active_session->contact.phone_number & 65535;
        }
        platform->debug_print("Connecting to RBC at "+ip+":"+std::to_string(port));
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            status = safe_radio_status::Failed;
            return;
        }
#ifdef _WIN32
        char t;
#else
        int t;
#endif
        t = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &t, sizeof(t));
        t = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &t, sizeof(t));
        #ifndef _WIN32
        t = 3;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &t, sizeof(t));
        t = 12;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &t, sizeof(t));
        t = 3;
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &t, sizeof(t));
        t = 20000;
        setsockopt(sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &t, sizeof(t));
        t = 1416;
        setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &t, sizeof(t));
        #endif
        socket = std::make_unique<TcpSocket>(sock, poller);
        socket->connect(ip, port);
        rx_promise = socket->receive().then(std::bind(&tcp_safe_connection::data_receive, this, std::placeholders::_1));
    }
    if ((setting_up && get_milliseconds() - connect_time > 40000) || (socket != nullptr && socket->is_failed())) {
        socket = nullptr;
        rx_promise = {};
        status = safe_radio_status::Failed;
    }
    if (status == safe_radio_status::Disconnected && socket && socket->is_connected()) {
        status = safe_radio_status::Connected;
        setting_up = false;
    }
}
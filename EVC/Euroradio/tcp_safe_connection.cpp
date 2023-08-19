/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "tcp_safe_connection.h"
#include "console_platform.h"
#include "session.h"
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

    if (session->contact.phone_number == 0xFFFFFFFFFFFFFFFFUL) {
        connect({"", ""});
    } else {
        dns_query = static_cast<ConsolePlatform*>(platform.get())->query_dns(hostname);
        dns_query->promise.then([this](dns_entry &&e){
            connect(std::move(e));
        });
    }
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
    dns_query = {};
    status = safe_radio_status::Disconnected;
}

void tcp_safe_connection::connect(dns_entry &&dns)
{
    dns_query = {};
    std::string ip;
    int port;
    std::map<std::string, std::string> config;
    {
        std::vector<std::string> params;
        std::string s = dns.txt;
        for (;;) {
            size_t pos = s.find_first_of(';');
            if (pos == std::string::npos) {
                if (!s.empty())
                    params.push_back(s);
                break;
            }
            params.push_back(s.substr(0,pos));
            s = s.substr(pos+1);
        }
        for (std::string &p : params) {
            size_t pos = p.find('=');
            if (pos != std::string::npos)
                config[p.substr(0, pos)] = p.substr(pos+1);
        }
    }
    if (dns.a != "" && config["txm"] != "CS") {
        ip = dns.a;
        port = config.find("port") != config.end() ? stoi(config["port"]) : 30998;
    } else {
        // CSD GSM-R call
        // Use TCP anyway, but derive address from phone number
        if (active_session->contact.phone_number == 0xFFFFFFFFFFFFFFFFULL) {
            ip = "127.0.0.1";
            port = 30998;
        } else {
            uint64_t num=0;
            for (int i=15; i>=0; i--)
            {
                int c = (active_session->contact.phone_number>>(4*i))&15;
                if (c == 15)
                    continue;
                num = 10*num + c;
            }
            for (int i=3; i>=0; i--)
            {
                ip += std::to_string((num>>(i*8+16)) & 255);
                if (i>0) ip += ".";
            }
            port = active_session->contact.phone_number & 65535;
        }
    }
    platform->debug_print("Connecting to RBC at "+ip+":"+std::to_string(port));
    platform->debug_print("Connection parameters: "+dns.txt);
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
    int tp[5] = {12, 3, 3, 20000, 1416};
    {
        std::string tp_str = config["tp"];
        for (int i=0; i<5; i++) {
            size_t pos = tp_str.find_first_of(',');
            std::string sub = tp_str.substr(0,pos);
            if (!sub.empty())
                tp[i] = stoi(sub);
            if (pos == std::string::npos)
                break;
            tp_str = tp_str.substr(pos+1);
        }
    }
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &tp[0], sizeof(tp[0]));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &tp[1], sizeof(tp[1]));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &tp[2], sizeof(tp[2]));
    setsockopt(sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &tp[3], sizeof(tp[3]));
    setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &tp[4], sizeof(tp[4]));
    #endif
    socket = std::make_unique<TcpSocket>(sock, poller);
    socket->connect(ip, port);
    rx_promise = socket->receive().then(std::bind(&tcp_safe_connection::data_receive, this, std::placeholders::_1));
}

void tcp_safe_connection::update()
{
    safe_radio_connection::update();
    if (setting_up && socket == nullptr && get_milliseconds() - connect_time > 5000)
        connect({"",""});
    if ((setting_up && get_milliseconds() - connect_time > 40000) || (socket != nullptr && socket->is_failed())) {
        socket = nullptr;
        rx_promise = {};
        dns_query = {};
        status = safe_radio_status::Failed;
    }
    if (status == safe_radio_status::Disconnected && socket && socket->is_connected()) {
        status = safe_radio_status::Connected;
        setting_up = false;
    }
}
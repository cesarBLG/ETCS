/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "tcp_cfm.h"
#include "radio_connection.h"
#include "terminal.h"
#include <sstream>
#include <iomanip>
#include "ares_dns.h"
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
static FdPoller *poller;
std::map<etcs_id, radio_connection_mode> connection_modes;
std::string radio_domain = "etcs.vtrains.es";
uint16_t crc16(const uint8_t *ptr, size_t count)
{
    uint16_t crc = 0xFFFF;
	for (size_t i=0; i<count; i++)
	{
		crc = crc ^ (((uint16_t)ptr[i]) << 8);
        for (int j=0; j<8; j++) {
			if (crc & 0x8000)
				crc = crc << 1 ^ 0x1021;
			else
				crc = crc << 1;
        }
	}
	return crc;
}
ALE_header::ALE_header(const std::vector<unsigned char> &data)
{
    Length = (data[0]<<8)|data[1];
    Version = data[2];
    AppType = data[3];
    TSeqNo = (data[4]<<8)|data[5];
    NRFlag = data[6];
    PacketType = data[7];
    Checksum = (data[8]<<8)|data[9];
}
std::vector<unsigned char> ALE_header::get_bytes()
{
    std::vector<unsigned char> data;
    data.push_back(Length >> 8);
    data.push_back(Length & 255);
    data.push_back(Version);
    data.push_back(AppType);
    data.push_back(TSeqNo >> 8);
    data.push_back(TSeqNo & 255);
    data.push_back(NRFlag);
    data.push_back(PacketType);
    Checksum = crc16(data.data(), 8);
    data.push_back(Checksum >> 8);
    data.push_back(Checksum & 255);
    return data;
}

ALE_header tcp_cfm::fill_ALE(std::vector<unsigned char> &data, int type)
{
    ALE_header header;
    header.Version = 0;
    header.AppType = peer_address.id.type == 1 ? 16 : 17;
    header.TSeqNo = 0;
    if (type == 1)
        seqno_tx = 0;
    header.TSeqNo = seqno_tx++;
    header.NRFlag = 1;
    header.PacketType = type;
    header.Length = header.PacketType == 1 ? (data.size() + 17) : (data.size() + 8);
    return header;
}

void tcp_cfm::T_connect_request(called_address address, etcs_id calling_address, std::vector<unsigned char> &&data)
{
    bool registered = false;
    for (auto *terminal : mobile_terminals) {
        if (terminal->registered) {
            registered = true;
            break;
        }
    }
    if (!registered) {
        handle_error(8, 1);
        return;
    }
    state = cfm_state::Connecting;
    own_id = calling_address;
    peer_address = address;
    au1 = std::move(data);
    if ((peer_address.id.id & 0x3FFF) == 0x3FFF) {
        poll_dns = true;
        connect_dns({"", ""});
    } else {
        if (connection_modes[peer_address.id] == radio_connection_mode::CS) {
            poll_dns = true;
            connect_dns({"", ""});
        } else {
            mobile_terminal *t = nullptr;
            for (auto *terminal : mobile_terminals) {
                if (terminal->mobile_data_available) {
                    terminal->ps_connections.push_back(weak_from_this());
                    t = terminal;
                    break;
                }
            }
            if (t != nullptr) {
                dns_query = query_dns();
                dns_query->promise.then([this](dns_entry &&e){
                    connect_dns(std::move(e));
                });
                dns_timer = platform->delay(5000).then([this]() {
                    connect_dns({"", ""});
                });
            } else {
                connect_dns({"", ""});
            }
        }
    }
}

std::unique_ptr<DNSQuery> tcp_cfm::query_dns()
{
    std::string hostname = "id";
    std::stringstream ss;
    ss << std::setfill ('0') << std::setw(6) << std::hex << peer_address.id.id;
    hostname += ss.str();
    ss = std::stringstream();
    ss << std::setfill ('0') << std::setw(2) << std::hex << peer_address.id.type,
    hostname += ".ty";
    hostname += ss.str();
    hostname += ".";
    hostname += radio_domain;

    platform->debug_print("Trying RBC "+hostname);

    return std::make_unique<AresQuery>(poller, hostname);
}

void tcp_cfm::T_data_request(std::vector<unsigned char> &&data)
{
    ALE_header header = fill_ALE(data, 3);
    auto ale = header.get_bytes();
    data.insert(data.begin(), ale.begin(), ale.end());
    socket->send(std::string((char*)data.data(), data.size()));
}

void tcp_cfm::T_disconnect_request(std::vector<unsigned char> &&data)
{
    ALE_header header = fill_ALE(data, 4);
    auto ale = header.get_bytes();
    data.insert(data.begin(), ale.begin(), ale.end());
    socket->send(std::string((char*)data.data(), data.size()));
    shutdown();
}

void tcp_cfm::data_received(std::string &&data)
{
    if (data.empty()) {
        if (state == cfm_state::Releasing) {
            socket = nullptr;
            rx_promise = {};
            state = cfm_state::Released;
        } else {
            handle_error(1, 3);
        }
        return;
    }

    rx_promise = socket->receive().then(std::bind(&tcp_cfm::data_received, this, std::placeholders::_1));

    if (rx_buffer.empty())
        rx_buffer = std::move(data);
    else
        rx_buffer += std::move(data);

    while (rx_buffer.size() >= 2) {
        size_t size = ((rx_buffer[0]&255)<<8)|(rx_buffer[1]&255);
        if (rx_buffer.size() < size + 2)
            return;

        std::vector<unsigned char> ale;
        ale.insert(ale.end(), rx_buffer.begin(), rx_buffer.begin() + size + 2);
        rx_buffer.erase(0, size + 2);
        parse_ALE(std::move(ale));
    }
}
void tcp_cfm::parse_ALE(std::vector<unsigned char> &&ale)
{
    ALE_header header(ale);
    uint16_t checksum = crc16(ale.data(), 8);
    if (header.Checksum != checksum) {
        platform->debug_print("Bad checksum");
        handle_error(7, 2);
        return;
    }
    ale.erase(ale.begin(), ale.begin() + 10);
    if (header.AppType != (peer_address.id.type == 1 ? 16 : 17)) {
        handle_error(6, 1);
        return;
    }
    if (header.PacketType == 2) {
        uint32_t responder_id = (ale[0]<<24)|(ale[1]<<16)|(ale[2]<<8)|ale[3];
        unsigned int type = responder_id>>24;
        peer_address.id = {type, responder_id&16777215};
        ale.erase(ale.begin(), ale.begin() + 4);
        state = cfm_state::Connected;
        T_connect_indication(peer_address.id, std::move(ale));
    } else if (header.PacketType == 3) {
        T_data_indication(std::move(ale));
    } else if (header.PacketType == 4) {
        T_disconnect_indication(std::move(ale));
        shutdown();
    }
}

void tcp_cfm::connect_dns(dns_entry &&dns)
{
    dns_query = {};
    dns_timer = {};
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
    if (dns.a != "") {
        std::string ip = dns.a;
        int port = config.find("port") != config.end() ? stoi(config["port"]) : 0x7911;
        connection_modes[peer_address.id] = radio_connection_mode::PS;
        platform->debug_print("Connecting to RBC at "+ip+":"+std::to_string(port));
        params = {ip, port, config["tp"]};
        connection_mode = radio_connection_mode::PS;
        connect();
    } else {
        if (connection_modes[peer_address.id] == radio_connection_mode::PS && config["txm"] != "CS") {
            handle_error(1, 6);
            return;
        }
        mobile_terminal *t = nullptr;
        for (mobile_terminal *terminal : mobile_terminals) {
            for (auto it = terminal->ps_connections.begin(); it != terminal->ps_connections.end();){
                auto *ptr = it->lock().get();
                if (ptr == this)
                    it = terminal->ps_connections.erase(it);
                else
                    ++it;
            }
        }
        for (mobile_terminal *terminal : mobile_terminals) {
            if (terminal->csd_available && !terminal->cs_connection) {
                terminal->cs_connection = weak_from_this();
                t = terminal;
                break;
            }
        }
        if (t == nullptr) {
            handle_error(2, 1);
            return;
        }
        // CSD GSM-R call
        connection_mode = radio_connection_mode::CS;
        connection_modes[peer_address.id] = radio_connection_mode::CS;
        // Use TCP anyway, but derive address from phone number
        // as CS mode is unsupported
        std::string ip;
        int port;
        if (peer_address.phone_number == 0xFFFFFFFFFFFFFFFFULL) {
            ip = "127.0.0.1";
            port = 0x7911;
        } else {
            uint64_t num=0;
            for (int i=15; i>=0; i--)
            {
                int c = (peer_address.phone_number>>(4*i))&15;
                if (c == 15)
                    continue;
                num = 10*num + c;
            }
            for (int i=3; i>=0; i--)
            {
                ip += std::to_string((num>>(i*8+16)) & 255);
                if (i>0) ip += ".";
            }
            port = num & 65535;
        }
        platform->debug_print("Connecting to RBC at "+ip+":"+std::to_string(port)+" (emulated CSD)");
        params = {ip, port, ""};
        connect();
    }
    if (dns.txt != "")
        platform->debug_print("Connection parameters: "+dns.txt);
}
void tcp_cfm::connect()
{
    addrinfo hints = {}, *res = nullptr;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(std::string(params.ip).c_str(), std::to_string(params.port).c_str(), &hints, &res);
    int sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        handle_error(1, 3);
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
        std::string tp_str = params.tp;
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
#ifdef __linux__
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &tp[0], sizeof(tp[0]));
    setsockopt(sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &tp[3], sizeof(tp[3]));
#endif
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &tp[1], sizeof(tp[1]));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &tp[2], sizeof(tp[2]));
    setsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &tp[4], sizeof(tp[4]));
#endif
	unsigned long one = 1;
#ifdef _WIN32
	ioctlsocket(sock, FIONBIO, &one);
#else
	ioctl(sock, FIONBIO, &one);
#endif
	::connect(sock, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

    socket = std::make_unique<TcpSocket>(sock, poller);
    rx_buffer.clear();
    rx_promise = socket->receive().then(std::bind(&tcp_cfm::data_received, this, std::placeholders::_1));
}

void tcp_cfm::handle_error(int reason, int subreason)
{
    cfm::handle_error(reason, subreason);
    dns_query = {};
    dns_timer = {};
    rx_promise = {};
    socket = nullptr;
    state = cfm_state::Released;
}

void tcp_cfm::shutdown()
{
    dns_query = {};
    dns_timer = {};
    if (socket == nullptr) {
        state = cfm_state::Released;
    } else {
        state = cfm_state::Releasing;
        socket->shutdown();
    }
}

void tcp_cfm::update()
{
    cfm::update();
    if (socket != nullptr && !au1.empty()) {
        std::vector<unsigned char> data = std::move(au1);
        au1.clear();
        ALE_header header = fill_ALE(data, 1);
        auto ale = header.get_bytes();
        ale.push_back(own_id.type);
        for (int i=2; i>=0; i--)
            ale.push_back((own_id.id>>(8*i))&255);
        ale.push_back(peer_address.id.type);
        for (int i=2; i>=0; i--)
            ale.push_back((peer_address.id.id>>(8*i))&255);
        ale.push_back(0);
        data.insert(data.begin(), ale.begin(), ale.end());
        socket->send(std::string((char*)data.data(), data.size()));
    }
    for (mobile_terminal *terminal : mobile_terminals) {
        for (auto conn : terminal->ps_connections) {
            auto *ptr = conn.lock().get();
            if (ptr == this && !terminal->mobile_data_available) {
                if (connection_mode == radio_connection_mode::PS)
                    handle_error(1, 1);
                dns_query = {};
                polling_dns_query = {};
            }
        }
        if (terminal->cs_connection && terminal->cs_connection->lock().get() == this && !terminal->csd_available) {
            handle_error(1, 1);
        }
    }
    if ((peer_address.id.id & 0x3FFF) != 0x3FFF && poll_dns && polling_dns_query == nullptr) {
        for (mobile_terminal *t : mobile_terminals) {
            if (t->mobile_data_available) {
                auto ptr = weak_from_this();
                etcs_id id = peer_address.id;
                t->ps_connections.push_back(ptr);
                polling_dns_query = query_dns();
                polling_dns_query->promise.then([ptr,id,t](dns_entry &&dns) {
                    if (dns.a != "") {
                        connection_modes[id] = radio_connection_mode::PS;
                    }
                    for (auto it = t->ps_connections.begin(); it != t->ps_connections.end();){
                        if (it->lock().get() == ptr.lock().get())
                            it = t->ps_connections.erase(it);
                        else
                            ++it;
                    }
                });
                break;
            }
        }
    }
}
void initialize_cfm(FdPoller &po)
{
	ares_library_init(ARES_LIB_INIT_ALL);
    poller = &po;
    get_cfm_handle = [](std::weak_ptr<safe_radio_connection> conn)
    {
        auto cfm = std::make_shared<tcp_cfm>(conn, *poller);
        cfm_connections.insert(cfm);
        return cfm;
    };
    platform->on_quit().then([](){
        ares_library_cleanup();
    }).detach();
    auto contents = platform->read_file("radio_domain.conf");
    if (contents)
        radio_domain = contents->substr(0, contents->find('\n'));
}
/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "cfm.h"
#include "tcp_socket.h"
#include "dns.h"
#include "console_platform.h"
enum struct radio_connection_mode
{
    None,
    CS,
    PS,
};
struct ALE_header
{
    uint16_t Length;
    uint8_t Version;
    uint8_t AppType;
    uint16_t TSeqNo;
    uint8_t NRFlag;
    uint8_t PacketType;
    uint16_t Checksum;
    ALE_header() {}
    ALE_header(const std::vector<unsigned char> &data);
    std::vector<unsigned char> get_bytes();
};

class tcp_cfm : public cfm
{
    struct tcp_params
    {
        std::string ip;
        int port;
        std::string tp;
    };
    etcs_id own_id;
    called_address peer_address;
    radio_connection_mode connection_mode = radio_connection_mode::None;
    PlatformUtil::Promise<void> dns_timer;
    std::unique_ptr<DNSQuery> dns_query;
    bool poll_dns=false;
    std::unique_ptr<DNSQuery> polling_dns_query;
    std::unique_ptr<TcpSocket> socket;
    PlatformUtil::Promise<std::string> rx_promise;
    std::string rx_buffer;
    FdPoller &poller;
    tcp_params params;
    int seqno_rx;
    int seqno_tx;
    std::vector<unsigned char> au1;
    void data_received(std::string &&msg);
    void parse_ALE(std::vector<unsigned char> &&data);
	ALE_header fill_ALE(std::vector<unsigned char> &data, int type);
    void connect();
    void connect_dns(dns_entry &&e);
    std::unique_ptr<DNSQuery> query_dns();
    void handle_error(int reason, int subreason) override;
public:
	tcp_cfm(std::weak_ptr<safe_radio_connection> conn, FdPoller &poller)
    : cfm(conn), poller(poller)
	{
	}
	void T_connect_request(called_address address, etcs_id calling_address, std::vector<unsigned char> &&data) override;
    void T_data_request(std::vector<unsigned char> &&ale) override;
    void T_disconnect_request(std::vector<unsigned char> &&data) override;
    void update() override;
};
void initialize_cfm(BasePlatform *pl, FdPoller &po);
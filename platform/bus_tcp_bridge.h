 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "tcp_listener.h"
#include "bus_socket_impl.h"

class BusTcpBridge : private PlatformUtil::NoCopy {
private:
    class BridgedTcpSocket : private PlatformUtil::NoCopy {
    private:
        std::unique_ptr<BasePlatform::BusSocket> bus_socket;
        PlatformUtil::Promise<std::pair<BasePlatform::BusSocket::PeerId, std::string>> bus_rx_promise;

        std::unique_ptr<TcpSocket> tcp_socket;
        PlatformUtil::Promise<std::string> tcp_rx_promise;
        std::string tcp_rx_buffer;

        std::optional<uint32_t> bus_tid;
        bool newline_framing;

        void tcp_rx(std::string &&data);
        void bus_rx(std::pair<BasePlatform::BusSocket::PeerId, std::string> &&data);

        bool alive;

    public:
        BridgedTcpSocket(std::unique_ptr<BasePlatform::BusSocket> &&bus, std::unique_ptr<TcpSocket> &&tcp, std::optional<uint32_t> tid, bool nl);
        bool is_alive() const;
    };

    void on_new_client(std::unique_ptr<TcpSocket> &&sock);

    std::string bus;
    BusSocketImpl& bus_impl;
    uint32_t rx_tid;
    std::optional<uint32_t> tx_tid;
    bool newline_framing;

	TcpListener listener;
    PlatformUtil::Promise<std::unique_ptr<TcpSocket>> accept_promise;
    std::vector<std::unique_ptr<BridgedTcpSocket>> clients;

public:
    BusTcpBridge(const std::string &bus, uint32_t rx_tid, std::optional<uint32_t> tx_tid, bool nl, const std::string hostname, int port, FdPoller &fd, BusSocketImpl& b);
};

class BusTcpBridgeManager : private PlatformUtil::NoCopy {
private:
    std::vector<std::unique_ptr<BusTcpBridge>> bridges;
public:
    BusTcpBridgeManager(const std::string &load_path, FdPoller &fd, BusSocketImpl &impl);
};

 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>
#include <atomic>
#include <optional>
#include <functional>
#include "tcp_listener.h"
#include "bus_socket_impl.h"

class BusTcpBridge : private PlatformUtil::NoCopy {
private:
    class BridgedTcpSocket : private PlatformUtil::NoCopy {
    private:
        std::unique_ptr<BasePlatform::BusSocket> bus_socket;
        PlatformUtil::Promise<std::pair<BasePlatform::BusSocket::ClientId, std::string>> bus_rx_promise;

        TcpSocket tcp_socket;
        PlatformUtil::Promise<std::string> tcp_rx_promise;
        std::string tcp_rx_buffer;

        std::optional<uint32_t> bus_tid;

        void tcp_rx(std::string &&data);
        void bus_rx(std::pair<BasePlatform::BusSocket::ClientId, std::string> &&data);

        bool alive;

    public:
        BridgedTcpSocket(std::unique_ptr<BasePlatform::BusSocket> &&bus, TcpSocket &&tcp, std::optional<uint32_t> tid);
        bool is_alive() const;
    };

    void on_new_client(TcpSocket &&sock);

    std::string bus;
    BusSocketImpl& bus_impl;
    uint32_t rx_tid;
    std::optional<uint32_t> tx_tid;

	TcpListener listener;
    PlatformUtil::Promise<TcpSocket> accept_promise;
    std::vector<std::unique_ptr<BridgedTcpSocket>> clients;

public:
    BusTcpBridge(const std::string &bus, uint32_t rx_tid, std::optional<uint32_t> tx_tid, const std::string hostname, int port, FdPoller &fd, BusSocketImpl& b);
};

class BusTcpBridgeManager : private PlatformUtil::NoCopy {
private:
    std::vector<std::unique_ptr<BusTcpBridge>> bridges;
public:
    BusTcpBridgeManager(const std::string &load_path, FdPoller &fd, BusSocketImpl &impl);
};

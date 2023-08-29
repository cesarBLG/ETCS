 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <functional>
#include <fstream>
#include <sstream>
#include <map>
#include "bus_tcp_bridge.h"

void BusTcpBridge::BridgedTcpSocket::tcp_rx(std::string &&data)
{
    if (data.empty()) {
        alive = false;
        bus_socket = nullptr;
        return;
    }

    tcp_rx_promise = tcp_socket->receive().then(std::bind(&BridgedTcpSocket::tcp_rx, this, std::placeholders::_1));

    if (tcp_rx_buffer.empty())
        tcp_rx_buffer = std::move(data);
    else
        tcp_rx_buffer += std::move(data);

    if (newline_framing) {
        size_t it;
        while ((it = tcp_rx_buffer.find_first_of("\r\n")) != -1) {
            if (it != 0) {
                if (bus_tid)
                    bus_socket->broadcast(*bus_tid, tcp_rx_buffer.substr(0, it));
                else
                    bus_socket->broadcast(tcp_rx_buffer.substr(0, it));
            }
            tcp_rx_buffer.erase(0, it);
            size_t i = 0;
            while (i < tcp_rx_buffer.size() && (tcp_rx_buffer[i] == '\r' || tcp_rx_buffer[i] == '\n'))
                tcp_rx_buffer.erase(0, 1);
        }
    } else {
        if (bus_tid)
            bus_socket->broadcast(*bus_tid, tcp_rx_buffer);
        else
            bus_socket->broadcast(tcp_rx_buffer);
        tcp_rx_buffer.clear();
    }
}

void BusTcpBridge::BridgedTcpSocket::bus_rx(BasePlatform::BusSocket::ReceiveResult &&result)
{
    if (!alive)
        return;

    bus_rx_promise = bus_socket->receive().then(std::bind(&BridgedTcpSocket::bus_rx, this, std::placeholders::_1));

    if (!std::holds_alternative<BasePlatform::BusSocket::Message>(result))
        return;
    auto msg = std::move(std::get<BasePlatform::BusSocket::Message>(result));

    if (newline_framing)
        tcp_socket->send(msg.data + "\r\n");
    else
        tcp_socket->send(msg.data);
}

BusTcpBridge::BridgedTcpSocket::BridgedTcpSocket(std::unique_ptr<BasePlatform::BusSocket> &&bus, std::unique_ptr<TcpSocket> &&tcp, std::optional<uint32_t> tid, bool nl) :
    bus_socket(std::move(bus)), tcp_socket(std::move(tcp)), bus_tid(tid), newline_framing(nl), alive(true)
{
    tcp_rx_promise = tcp_socket->receive().then(std::bind(&BridgedTcpSocket::tcp_rx, this, std::placeholders::_1));
    bus_rx_promise = bus_socket->receive().then(std::bind(&BridgedTcpSocket::bus_rx, this, std::placeholders::_1));
}

bool BusTcpBridge::BridgedTcpSocket::is_alive() const {
    return alive;
}

void BusTcpBridge::on_new_client(std::unique_ptr<TcpSocket> &&sock)
{
    accept_promise = listener.accept().then(std::bind(&BusTcpBridge::on_new_client, this, std::placeholders::_1));

    std::unique_ptr<BasePlatform::BusSocket> bus_socket = bus_impl.open_bus_socket(bus, rx_tid);
    if (!bus_socket)
        return;

    clients.erase(std::remove_if(clients.begin(), clients.end(), [](const auto &c){ return !c->is_alive(); }), clients.end());
    clients.push_back(std::make_unique<BridgedTcpSocket>(std::move(bus_socket), std::move(sock), tx_tid, newline_framing));
}

BusTcpBridge::BusTcpBridge(const std::string_view bus, uint32_t rx_tid, std::optional<uint32_t> tx_tid, bool nl, const std::string hostname, int port, FdPoller &fd, BusSocketImpl& b) :
    bus(bus), listener(hostname, port, fd), rx_tid(rx_tid), tx_tid(tx_tid), newline_framing(nl), bus_impl(b)
{
    accept_promise = listener.accept().then(std::bind(&BusTcpBridge::on_new_client, this, std::placeholders::_1));
}

BusTcpBridgeManager::BusTcpBridgeManager(const std::string_view load_path, FdPoller &fd, BusSocketImpl &impl)
{
    std::ifstream file(std::string(load_path) + "tcp_bus_bridge.conf", std::ios::binary);
    std::string line;

    std::map<std::string, std::string> ini_items;
    while (std::getline(file, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        int pos = line.find('=');
        if (pos == -1)
            continue;
        ini_items.insert(std::pair<std::string, std::string>(line.substr(0, pos), line.substr(pos+1)));
    }

    for (const auto &entry : ini_items) {
        std::istringstream key1(entry.first);
        std::istringstream key2(entry.second);
        std::string tcphost, tcpport;
        std::string busname, rx_tid, tx_tid, nl;
        std::getline(key1, tcphost, ':');
        std::getline(key1, tcpport, ':');
        std::getline(key2, busname, ':');
        std::getline(key2, rx_tid, ':');
        std::getline(key2, tx_tid, ':');
        std::getline(key2, nl, ':');
        uint32_t rxt = BasePlatform::BusSocket::PeerId::fourcc(rx_tid);
        std::optional<uint32_t> txt;
        if (tx_tid != "*")
            txt = BasePlatform::BusSocket::PeerId::fourcc(tx_tid);
        bool newline_framing = (!nl.empty() && nl[0] == 'n');
        bridges.push_back(std::make_unique<BusTcpBridge>(busname, rxt, txt, newline_framing, tcphost, std::stoi(tcpport), fd, impl));
    }
}

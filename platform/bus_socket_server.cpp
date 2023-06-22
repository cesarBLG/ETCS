 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <map>
#include <functional>
#include <fstream>
#include <sstream>
#include "bus_socket_server.h"

void BusSocketServer::pack_uint32(char* ptr, uint32_t v) {
    unsigned char* p = (unsigned char*)ptr;
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

uint32_t BusSocketServer::unpack_uint32(const char* ptr) {
    unsigned char* p = (unsigned char*)ptr;
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

void BusSocketServer::handle_client(ClientData &client)
{
    while (true) {
        if (client.rx_buffer.size() < 1 * 4)
            return;
        uint32_t msgtype = unpack_uint32(client.rx_buffer.data() + 0 * 4);
        if (msgtype == 11) { // hello
            if (client.rx_buffer.size() < 2 * 4)
                return;
            if (!client.tid.has_value()) {
                client.tid = unpack_uint32(client.rx_buffer.data() + 1 * 4);

                {
                    std::string buf;
                    buf.resize(3 * 4);
                    pack_uint32(buf.data() + 0 * 4, 1); // join
                    pack_uint32(buf.data() + 1 * 4, *client.tid);
                    pack_uint32(buf.data() + 2 * 4, client.uid);
                    for (ClientData &cl : clients)
                        if (&cl != &client && cl.tid.has_value())
                            cl.socket->send(buf);
                }

                for (ClientData &cl : clients) {
                    if (&cl == &client || !cl.tid.has_value())
                        continue;
                    std::string buf;
                    buf.resize(3 * 4);
                    pack_uint32(buf.data() + 0 * 4, 1); // join
                    pack_uint32(buf.data() + 1 * 4, *cl.tid);
                    pack_uint32(buf.data() + 2 * 4, cl.uid);
                    client.socket->send(buf);
                }
            }
            client.rx_buffer.erase(0, 2 * 4);
        } else if (msgtype == 12) { // broadcast all
            if (client.rx_buffer.size() < 2 * 4)
                return;
            uint32_t len = unpack_uint32(client.rx_buffer.data() + 1 * 4);
            if (client.rx_buffer.size() < 2 * 4 + len)
                return;
            if (client.tid.has_value()) {
                std::string buf;
                buf.reserve(4 * 4 + len);
                buf.resize(4 * 4);
                pack_uint32(buf.data() + 0 * 4, 3); // data
                pack_uint32(buf.data() + 1 * 4, *client.tid);
                pack_uint32(buf.data() + 2 * 4, client.uid);
                pack_uint32(buf.data() + 3 * 4, len);
                buf.insert(buf.end(), client.rx_buffer.begin() + 2 * 4, client.rx_buffer.begin() + 2 * 4 + len);

                for (ClientData &cl : clients)
                    if (&cl != &client && cl.tid.has_value())
                        cl.socket->send(buf);
            }
            client.rx_buffer.erase(0, 2 * 4 + len);
        } else if (msgtype == 13) { // broadcast tid
            if (client.rx_buffer.size() < 3 * 4)
                return;
            uint32_t tid = unpack_uint32(client.rx_buffer.data() + 1 * 4);
            uint32_t len = unpack_uint32(client.rx_buffer.data() + 2 * 4);
            if (client.rx_buffer.size() < 3 * 4 + len)
                return;
            if (client.tid.has_value()) {
                std::string buf;
                buf.reserve(4 * 4 + len);
                buf.resize(4 * 4);
                pack_uint32(buf.data() + 0 * 4, 3); // data
                pack_uint32(buf.data() + 1 * 4, *client.tid);
                pack_uint32(buf.data() + 2 * 4, client.uid);
                pack_uint32(buf.data() + 3 * 4, len);
                buf.insert(buf.end(), client.rx_buffer.begin() + 3 * 4, client.rx_buffer.begin() + 3 * 4 + len);

                for (ClientData &cl : clients)
                    if (&cl != &client && cl.tid == tid && cl.tid.has_value())
                        cl.socket->send(buf);
            }
            client.rx_buffer.erase(0, 3 * 4 + len);
        } else if (msgtype == 14) { // send to
            if (client.rx_buffer.size() < 3 * 4)
                return;
            uint32_t uid = unpack_uint32(client.rx_buffer.data() + 1 * 4);
            uint32_t len = unpack_uint32(client.rx_buffer.data() + 2 * 4);
            if (client.rx_buffer.size() < 3 * 4 + len)
                return;
            if (client.tid.has_value()) {
                std::string buf;
                buf.reserve(4 * 4 + len);
                buf.resize(4 * 4);
                pack_uint32(buf.data() + 0 * 4, 3); // data
                pack_uint32(buf.data() + 1 * 4, *client.tid);
                pack_uint32(buf.data() + 2 * 4, client.uid);
                pack_uint32(buf.data() + 3 * 4, len);
                buf.insert(buf.end(), client.rx_buffer.begin() + 3 * 4, client.rx_buffer.begin() + 3 * 4 + len);

                for (ClientData &cl : clients)
                    if (&cl != &client && cl.uid == uid && cl.tid.has_value())
                        cl.socket->send(buf);
            }
            client.rx_buffer.erase(0, 3 * 4 + len);
        } else {
            client.rx_buffer.clear();
        }
    }
}

void BusSocketServer::on_client_data(std::string &&data, uint32_t uid)
{
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].uid != uid)
            continue;

        if (data.empty()) {
            if (clients[i].tid.has_value()) {
                std::string buf;
                buf.resize(3 * 4);
                pack_uint32(buf.data() + 0 * 4, 2); // leave
                pack_uint32(buf.data() + 1 * 4, *clients[i].tid);
                pack_uint32(buf.data() + 2 * 4, clients[i].uid);

                for (ClientData &cl : clients)
                    if (&cl != &clients[i] && cl.tid.has_value())
                        cl.socket->send(buf);
            }

            clients.erase(clients.begin() + i);
        } else {
            clients[i].rx_promise = clients[i].socket->receive().then(std::bind(&BusSocketServer::on_client_data, this, std::placeholders::_1, uid));
            clients[i].rx_buffer += std::move(data);
            handle_client(clients[i]);
        }
        return;
    }
}

void BusSocketServer::on_new_client(std::unique_ptr<TcpSocket> &&sock)
{
    accept_promise = listener.accept().then(std::bind(&BusSocketServer::on_new_client, this, std::placeholders::_1));
    uint32_t id = ++uid;
    clients.push_back(ClientData{ std::nullopt, id, std::move(sock), {} });
    clients.back().rx_promise = clients.back().socket->receive().then(std::bind(&BusSocketServer::on_client_data, this, std::placeholders::_1, id));
}

BusSocketServer::BusSocketServer(const std::string &hostname, int port, FdPoller &p) :
    listener(hostname, port, p),
    uid(0)
{
    accept_promise = listener.accept().then(std::bind(&BusSocketServer::on_new_client, this, std::placeholders::_1));
}

BusSocketServerManager::BusSocketServerManager(const std::string &load_path, FdPoller &fd)
{
    std::ifstream file(load_path + "tcp_bus_server.conf", std::ios::binary);
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
        std::istringstream key2(entry.second);
        std::string tcphost, tcpport;
        std::getline(key2, tcphost, ':');
        std::getline(key2, tcpport, ':');
        servers.push_back(std::make_unique<BusSocketServer>(tcphost, std::stoi(tcpport), fd));
    }
}

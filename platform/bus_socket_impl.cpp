 /*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bus_socket_impl.h"
#include <fstream>
#include <sstream>
#include <cstring>

BusSocketImpl::BusSocketImpl(const std::string &load_path, FdPoller &p) : poller(p) {
	std::ifstream file(load_path + "tcp_bus_client.conf", std::ios::binary);
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
		std::istringstream key(entry.second);
		std::string delim3, delim4, delim5;
		std::getline(key, delim3, ':');
		std::getline(key, delim4, ':');
		socket_config.push_back(SocketConfig{entry.first, delim3, std::stoi(delim4)});
	}
}

std::unique_ptr<BasePlatform::BusSocket> BusSocketImpl::open_bus_socket(const std::string &channel, uint32_t tid) {
	for (const SocketConfig &conf : socket_config)
		if (conf.name == channel)
			return std::make_unique<TcpBusSocket>(tid, conf.hostname, conf.port, poller);
	platform->debug_print("unconfigured bus socket channel \"" + channel + "\"!");
	return nullptr;
}

void BusSocketImpl::TcpBusSocket::pack_uint32(char* ptr, uint32_t v) {
	unsigned char* p = (unsigned char*)ptr;
	p[0] = (unsigned char)(v);
	p[1] = (unsigned char)(v >> 8);
	p[2] = (unsigned char)(v >> 16);
	p[3] = (unsigned char)(v >> 24);
}

uint32_t BusSocketImpl::TcpBusSocket::unpack_uint32(const char* ptr) {
	unsigned char* p = (unsigned char*)ptr;
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

void BusSocketImpl::TcpBusSocket::data_received(std::string &&data) {
	if (data.empty()) {
		rx_buffer.clear();
		retry_promise = std::move(platform->delay(100).then([this](){
			socket = TcpSocket(hostname, port, poller);
			rx_promise = std::move(socket.receive().then(std::bind(&TcpBusSocket::data_received, this, std::placeholders::_1)));
			client_hello();
		}));
		return;
	}

	rx_promise = std::move(socket.receive().then(std::bind(&TcpBusSocket::data_received, this, std::placeholders::_1)));

	if (rx_buffer.empty())
		rx_buffer = std::move(data);
	else
		rx_buffer += data;

	while (true) {
		if (rx_buffer.size() < 3 * 4)
			return;
		uint32_t msgtype = unpack_uint32(rx_buffer.data() + 0 * 4);
		ClientId id = ClientId {
			unpack_uint32(rx_buffer.data() + 1 * 4),
			unpack_uint32(rx_buffer.data() + 2 * 4) };

		if (msgtype == 1) { // join
			on_join_list.fulfill_all(id);
			rx_buffer.erase(0, 3 * 4);
		}
		else if (msgtype == 2) { // leave
			on_leave_list.fulfill_all(id);
			rx_buffer.erase(0, 3 * 4);
		}
		else if (msgtype == 3) { // data
			if (rx_buffer.size() < 4 * 4)
				return;
			uint32_t len = unpack_uint32(rx_buffer.data() + 3 * 4);

			if (rx_buffer.size() < 4 * 4 + len)
				return;
			if (rx_buffer.size() - 4 * 4 == len) {
				rx_buffer.erase(0, 4 * 4);
				rx_list.fulfill_one(std::make_pair(id, std::move(rx_buffer)));
				rx_buffer.clear();
			} else {
				rx_list.fulfill_one(std::make_pair(id, rx_buffer.substr(4 * 4, len)));
				rx_buffer.erase(0, 4 * 4 + len);
			}
		} else {
			rx_buffer.clear();
		}
	}
}

BusSocketImpl::TcpBusSocket::TcpBusSocket(uint32_t tid, const std::string &hostname, int port, FdPoller &poller) :
	hostname(hostname), port(port), tid(tid), poller(poller), socket(hostname, port, poller) {
	rx_promise = std::move(socket.receive().then(std::bind(&TcpBusSocket::data_received, this, std::placeholders::_1)));
	client_hello();
}

void BusSocketImpl::TcpBusSocket::client_hello() {
	std::string buf;
	buf.resize(2 * 4);
	pack_uint32(buf.data() + 0 * 4, 11); // hello
	pack_uint32(buf.data() + 1 * 4, tid);
	socket.send(std::move(buf));
}

void BusSocketImpl::TcpBusSocket::broadcast(const std::string &data) {
	std::string buf;
	buf.reserve(2 * 4 + data.size());
	buf.resize(2 * 4);
	pack_uint32(buf.data() + 0 * 4, 12); // broadcast all
	pack_uint32(buf.data() + 1 * 4, data.size());
	buf.insert(buf.end(), data.begin(), data.end());
	socket.send(std::move(buf));
}

void BusSocketImpl::TcpBusSocket::broadcast(uint32_t tid, const std::string &data) {
	std::string buf;
	buf.reserve(3 * 4 + data.size());
	buf.resize(3 * 4);
	pack_uint32(buf.data() + 0 * 4, 13); // broadcast tid
	pack_uint32(buf.data() + 1 * 4, tid);
	pack_uint32(buf.data() + 2 * 4, data.size());
	buf.insert(buf.end(), data.begin(), data.end());
	socket.send(std::move(buf));
}

void BusSocketImpl::TcpBusSocket::send_to(uint32_t uid, const std::string &data) {
	std::string buf;
	buf.reserve(3 * 4 + data.size());
	buf.resize(3 * 4);
	pack_uint32(buf.data() + 0 * 4, 14); // send to
	pack_uint32(buf.data() + 1 * 4, uid);
	pack_uint32(buf.data() + 2 * 4, data.size());
	buf.insert(buf.end(), data.begin(), data.end());
	socket.send(std::move(buf));
}

PlatformUtil::Promise<std::pair<BasePlatform::BusSocket::ClientId, std::string>> BusSocketImpl::TcpBusSocket::receive() {
	return rx_list.create_and_add();
}

PlatformUtil::Promise<BasePlatform::BusSocket::ClientId> BusSocketImpl::TcpBusSocket::on_peer_join() {
	return on_join_list.create_and_add();
}

PlatformUtil::Promise<BasePlatform::BusSocket::ClientId> BusSocketImpl::TcpBusSocket::on_peer_leave() {
	return on_leave_list.create_and_add();
}

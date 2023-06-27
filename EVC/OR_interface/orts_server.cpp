#include "platform_runtime.h"
#include "orts_wrapper.h"
#include <orts/server.h>

std::unique_ptr<BasePlatform::BusSocket> srv_socket;
std::map<uint32_t, std::unique_ptr<ORTSClientWrapper>> srv_clients;
std::unique_ptr<ORserver::Server> srv_orts;

void orts_receive(BasePlatform::BusSocket::Message &&msg)
{
    auto it = srv_clients.find(msg.peer.uid);
    if (it != srv_clients.end())
        srv_orts->ParseLine(it->second.get(), std::move(msg.data));
}

void orts_receive(BasePlatform::BusSocket::JoinNotification &&msg)
{
    std::unique_ptr<ORTSClientWrapper> wrapper = std::make_unique<ORTSClientWrapper>(*srv_socket, msg.peer.uid, true);
    srv_orts->AddClient(wrapper.get());
    srv_clients.insert(std::make_pair(msg.peer.uid, std::move(wrapper)));
}

void orts_receive(BasePlatform::BusSocket::LeaveNotification &&msg)
{
    auto it = srv_clients.find(msg.peer.uid);
    if (it != srv_clients.end()) {
        srv_orts->RemoveClient(it->second.get());
        srv_clients.erase(it);
    }
}

void orts_receive_handler(BasePlatform::BusSocket::ReceiveResult &&result)
{
    srv_socket->receive().then(orts_receive_handler).detach();
    std::visit([](auto&& arg){ orts_receive(std::move(arg)); }, std::move(result));
}

void orts_start()
{
    auto conf = platform->read_file("orts_server.conf");
    if (!conf || conf->size() == 0 || conf->front() == '0')
        return;

    srv_socket = platform->open_socket("evc_sim", BasePlatform::BusSocket::PeerId::fourcc("SRV"));
    if (!srv_socket)
        return;
    srv_orts = std::make_unique<ORserver::Server>();
    srv_socket->receive().then(orts_receive_handler).detach();
}

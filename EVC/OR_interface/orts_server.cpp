#include "platform.h"
#include "orts_wrapper.h"
#include <orts/server.h>

std::unique_ptr<BasePlatform::BusSocket> srv_socket;
std::map<uint32_t, std::unique_ptr<ORTSClientWrapper>> srv_clients;
std::unique_ptr<ORserver::Server> srv_orts;

void orts_data_received(std::pair<BasePlatform::BusSocket::ClientId, std::string> &&data)
{
    srv_socket->receive().then(orts_data_received).detach();

    auto it = srv_clients.find(data.first.uid);
    if (it != srv_clients.end())
        srv_orts->ParseLine(it->second.get(), std::move(data.second));
}

void orts_peer_join(BasePlatform::BusSocket::ClientId peer)
{
    srv_socket->on_peer_join().then(orts_peer_join).detach();

    std::unique_ptr<ORTSClientWrapper> wrapper = std::make_unique<ORTSClientWrapper>(*srv_socket, peer.uid, true);
    srv_orts->AddClient(wrapper.get());
    srv_clients.insert(std::make_pair(peer.uid, std::move(wrapper)));
}

void orts_peer_leave(BasePlatform::BusSocket::ClientId peer)
{
    srv_socket->on_peer_leave().then(orts_peer_leave).detach();

    auto it = srv_clients.find(peer.uid);
    if (it != srv_clients.end()) {
        srv_orts->RemoveClient(it->second.get());
        srv_clients.erase(it);
    }
}

void orts_start()
{
    srv_socket = platform->open_socket("evc_sim", BasePlatform::BusSocket::ClientId::fourcc("SRV"));
    if (!srv_socket)
        return;
    srv_orts = std::make_unique<ORserver::Server>();
    srv_socket->receive().then(orts_data_received).detach();
    srv_socket->on_peer_join().then(orts_peer_join).detach();
    srv_socket->on_peer_leave().then(orts_peer_leave).detach();
}

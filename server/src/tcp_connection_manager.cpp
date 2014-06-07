
#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"

#include "log.hpp"

namespace geryon { namespace server {

std::shared_ptr<TCPConnection> TCPConnectionManager::start(boost::asio::ip::tcp::socket && socket,
                                                           boost::asio::io_service & ioservice) {
    std::shared_ptr<TCPConnection> c = create(std::move(socket), ioservice);
    c->start();
    if(trackConnections) {
        std::unique_lock<std::mutex> _(mutex);
        connections.insert(c);
        LOG(geryon::util::Log::DEBUG) << "Start: ConnMgr has " << connections.size() << " connections.";
    }
    return c;
}

void TCPConnectionManager::stop(std::shared_ptr<TCPConnection> c) {
    c->stop();
    if(trackConnections) {
        std::unique_lock<std::mutex> _(mutex);
        connections.erase(c);
        LOG(geryon::util::Log::DEBUG) << "Stop: ConnMgr has " << connections.size() << " connections.";
    }
}

void TCPConnectionManager::stopAll() {
    std::unique_lock<std::mutex> _(mutex);
    LOG(geryon::util::Log::DEBUG) << "Stop all: ConnMgr has " << connections.size() << " connections.";
    for(auto c : connections) {
        c->stop();
    }
    connections.clear();
}

} }

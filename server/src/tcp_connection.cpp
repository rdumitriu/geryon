/**
 * \file tcp_connection.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <boost/bind.hpp>

#include "tcp_connection.hpp"
#include "tcp_protocol_handler.hpp"

#include "log.hpp"

namespace geryon { namespace server {

TCPConnection::TCPConnection(boost::asio::ip::tcp::socket && _socket,
                             boost::asio::io_service & io_service,
                             TCPConnectionManager & _connectionManager,
                             std::shared_ptr<TCPProtocolHandler> _protocolHandler)
                        : strand(io_service),
                          socket(std::move(_socket)),
                          rConnectionManager(_connectionManager),
                          pProtocolHandler(_protocolHandler) {
}

TCPConnection::~TCPConnection() {
}

void TCPConnection::start() {
    try {
        protocolHandler()->init(*this);
    } catch( std::runtime_error & e) {
        LOG(geryon::util::Log::ERROR) << "Failed to initialize properly the protocol handler! Error was :" << e.what();
        stop(); //::TODO:: shouldn't be through connMgr ?
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Failed to initialize properly the protocol handler!";
        stop(); //::TODO:: shouldn't be through connMgr ?
    }
}

void TCPConnection::stop() {
    if(!socket.is_open()) {
        return;
    }
    boost::system::error_code ignored_ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    try {
        protocolHandler()->done();
    } catch( std::runtime_error & e) {
        LOG(geryon::util::Log::ERROR) << "Failed to cleanup properly the protocol handler! Error was :" << e.what();
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Failed to cleanup properly the protocol handler!";
    }
}

bool TCPConnection::scheduleRead(GBufferHandler && _readBuffer) {
    std::unique_lock<std::mutex> _(mutex);
    if(readBuffer.isValid()) {
        //read already in progress
        LOG(geryon::util::Log::ERROR) << "Protocol error, read already in progress?!";
        return false;
    }
    readBuffer = std::move(_readBuffer);
    //ok, we must request a read
    GBuffer buff = readBuffer.get();
    if(buff.size() == buff.marker()) {
        //read buffer is full ?!?
        LOG(geryon::util::Log::ERROR) << "Protocol error, read buffer is full?!";
        return false;
    }
    scheduleASIORead();
    LOG(geryon::util::Log::DEBUG) << "Read will be scheduled.";
    return true;
}


bool TCPConnection::scheduleWrite(GBufferHandler && _writeBuffer) {
    std::unique_lock<std::mutex> _(mutex);
    writeBuffers.push_back(std::move(_writeBuffer));
    scheduleASIOWrite(false);
    LOG(geryon::util::Log::DEBUG) << "Write will be scheduled.";
    return true;
}

bool TCPConnection::rescheduleWrite() {
    std::unique_lock<std::mutex> _(mutex);
    if(!writeBuffers.empty()) {
        writeBuffers.pop_front(); //get rid of the front buffer
    }
    //more work to do ?
    if(!writeBuffers.empty()) {
        LOG(geryon::util::Log::DEBUG) << "Write will be re-scheduled.";
        scheduleASIOWrite(true);
        return true;
    }
    return false;
}

} }  /* namespace */

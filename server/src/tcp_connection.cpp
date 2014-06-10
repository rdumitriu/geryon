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
                        : socket(std::move(_socket)),
                          rConnectionManager(_connectionManager),
                          pProtocolHandler(_protocolHandler),
                          asioOperationInProgress(false), acceptCommands(true) {
}

TCPConnection::~TCPConnection() {
}

void TCPConnection::start() {
    try {
        protocolHandler()->init(shared_this());
    } catch( std::runtime_error & e) {
        LOG(geryon::util::Log::ERROR) << "Failed to initialize properly the protocol handler! Error was :" << e.what();
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Failed to initialize properly the protocol handler!";
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

void TCPConnection::scheduleRead(GBufferHandler && _readBuffer) {
    detail::TCPConnectionCommandPtr ptr = std::make_shared<detail::TCPConnectionCommand>();
    ptr->command = detail::TCPConnectionCommand::READ;
    ptr->bufferHandler = std::move(_readBuffer);

    std::unique_lock<std::mutex> _(mutex);
    if(acceptCommands) {
        commands.push_back(ptr);
    }
    if(!asioOperationInProgress) {
        scheduleNextOperation();
    }
}


void TCPConnection::scheduleWrite(GBufferHandler && _writeBuffer) {
    detail::TCPConnectionCommandPtr ptr = std::make_shared<detail::TCPConnectionCommand>();
    ptr->command = detail::TCPConnectionCommand::WRITE;
    ptr->bufferHandler = std::move(_writeBuffer);

    std::unique_lock<std::mutex> _(mutex);
    if(acceptCommands) {
        commands.push_back(ptr);
    }
    if(!asioOperationInProgress) {
        scheduleNextOperation();
    }
}

void TCPConnection::scheduleClose() {
    detail::TCPConnectionCommandPtr ptr = std::make_shared<detail::TCPConnectionCommand>();
    ptr->command = detail::TCPConnectionCommand::CLOSE;

    std::unique_lock<std::mutex> _(mutex);
    if(acceptCommands) {
        commands.push_back(ptr);
        acceptCommands = false;
    }
    if(!asioOperationInProgress) {
        scheduleNextOperation();
    }
}

void TCPConnection::rescheduleASIO() {
    std::unique_lock<std::mutex> _(mutex);
    if(commands.empty()) {
        // WTH ?!?
        return;
    }
    commands.pop_front();
    asioOperationInProgress = false; //reset flag
    scheduleNextOperation();
}

void TCPConnection::scheduleNextOperation() {
    if(commands.empty()) {
        return;
    }
    asioOperationInProgress = true;
    switch(commands.front()->command) {
        case detail::TCPConnectionCommand::READ:
            scheduleASIORead();
            break;
        case detail::TCPConnectionCommand::WRITE:
            scheduleASIOWrite();
            break;
        case detail::TCPConnectionCommand::CLOSE:
        default:
            scheduleASIOClose();
            break;
    }
}

} }  /* namespace */

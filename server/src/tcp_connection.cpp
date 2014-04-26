/*
 * tcp_connection.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <boost/bind.hpp>

#include "tcp_connection.hpp"
#include "tcp_protocol_handler.hpp"

#include "log.hpp"

namespace geryon { namespace server {

TCPConnection::TCPConnection(boost::asio::io_service& io_service,
                             TCPProtocolHandler * const pProtocolHandler)
                        : strand(io_service),
                          socket(io_service),
                          protocolHandler(pProtocolHandler) {
}

TCPConnection::~TCPConnection() {
}

void TCPConnection::doCommunication() {
    protocolHandler->init();
    if(protocolHandler->isClientInitiated()) {
		readMore();
	} else {
		writeMore();
	}
}

boost::asio::ip::tcp::socket& TCPConnection::tcpSocket() {
    return socket;
}

void TCPConnection::readHandler(const boost::system::error_code& e,
								std::size_t nBytes) {
    if(!nBytes) {
        return;
    }
	if(e) {
        LOG(geryon::util::Log::ERROR) << "Error reading from "
                                      << socket.remote_endpoint().address().to_string();
		return;
	}
    TCPProtocolHandler::Operation op = protocolHandler->read(nBytes);
    handleNextEvent(op);
}

void TCPConnection::writeHandler(const boost::system::error_code& e,
								 std::size_t nBytes) {
    if(!nBytes) {
        return;
    }
	if(e) {
        LOG(geryon::util::Log::ERROR) << "Error writing to "
                                      << socket.remote_endpoint().address().to_string();
		return;
	}
    TCPProtocolHandler::Operation op = protocolHandler->write(nBytes);
    handleNextEvent(op);
}

void TCPConnection::closeHandler(const boost::system::error_code& e) {
    close();
}

void TCPConnection::handleNextEvent(TCPProtocolHandler::Operation op) {
    switch(op) {
        case TCPProtocolHandler::CLOSE:
            requestClose();
            break;
        case TCPProtocolHandler::READ:
            readMore();
            break;
        case TCPProtocolHandler::WRITE:
            writeMore();
            break;
    }
}

void TCPConnection::readMore() {
    LOG(geryon::util::Log::DEBUG) << "readMore() called";

    socket.async_read_some(protocolHandler->readBuffer(),
                           strand.wrap(boost::bind(&TCPConnection::readHandler,
                                                   shared_from_this(),
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred)));
    LOG(geryon::util::Log::DEBUG) << "readMore() called, read requested";
}

void TCPConnection::writeMore() {
    LOG(geryon::util::Log::DEBUG) << "writeMore() called";
    socket.async_write_some(protocolHandler->writeBuffer(),
                            strand.wrap(boost::bind(&TCPConnection::writeHandler,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred)));
    LOG(geryon::util::Log::DEBUG) << "writeMore() called, write requested";
}

void TCPConnection::requestClose() {
    boost::asio::async_write(socket,
                             boost::asio::buffer(boost::asio::mutable_buffer(NULL, 0)),
                             strand.wrap(boost::bind(&TCPConnection::closeHandler,
                                                     shared_from_this(),
                                                     boost::asio::placeholders::error)));
    LOG(geryon::util::Log::DEBUG) << "requestClose() called";
}


void TCPConnection::close() {
    boost::system::error_code ignored_ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    protocolHandler->done();
}

} }  /* namespace */

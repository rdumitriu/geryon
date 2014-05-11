/**
 * \file tcp_protocol_handler.cpp
 *
 *  Created on: Apr 12, 2014
 *      Author: rdumitriu
 */
#include "tcp_connection.hpp"
#include "tcp_protocol_handler.hpp"

#include "log.hpp"

namespace geryon { namespace server {


void TCPProtocolHandler::requestRead(GBufferHandler && readBuffer) {
    getConnection().scheduleRead(std::move(readBuffer));
}

void TCPProtocolHandler::requestWrite(GBufferHandler && writeBuffer) {
    getConnection().scheduleWrite(std::move(writeBuffer));
}

void TCPProtocolHandler::requestClose() {
    getConnection().scheduleClose();
}

} }

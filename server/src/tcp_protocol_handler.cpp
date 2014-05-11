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


bool TCPProtocolHandler::requestRead(GBufferHandler && readBuffer) {
    return getConnection().scheduleRead(std::move(readBuffer));
}

bool TCPProtocolHandler::requestWrite(GBufferHandler && writeBuffer) {
    return getConnection().scheduleWrite(std::move(writeBuffer));
}

bool TCPProtocolHandler::requestClose() {
    GBufferHandler invalid;
    return getConnection().scheduleWrite(std::move(invalid));
}

} }

///
/// \file http_protocol.cpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#include "http_protocol.hpp"
#include "http_protocol_handler.hpp"
#include "server_global_structs.hpp"

#include "string_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {

std::shared_ptr<TCPProtocolHandler> HttpProtocol::createHandler() {
    return std::make_shared<HttpProtocolHandler>(ServerGlobalStucts::getMemoryPool().get(),
                                                 executor,
                                                 maxRequestLenght);
}


} }

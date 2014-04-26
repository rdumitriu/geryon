/*
 * HTTPServer.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */

#include "http_server.hpp"

namespace geryon { namespace server {
 

HTTPServer::HTTPServer(const TCPServerConfig * sc,
                       std::size_t thread_pool_size) 
                                : TCPServer(sc, thread_pool_size) {                           
}

TCPConnection * HTTPServer::createTCPConnection(boost::asio::io_service & iosrvc) {
    return new StandardTCPConnection(iosrvc, new HTTPProtocolHandler());
}

} } /* namespace */

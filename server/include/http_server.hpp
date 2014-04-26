/*
 * HTTPServer.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */
#include <string>

#include "tcp_server.hpp"

#ifndef HTTPSERVER_HPP_
#define HTTPSERVER_HPP_

namespace geryon { namespace server {
 
class HTTPServer : public TCPServer {
public:
    explicit HTTPServer(const std::string & _srvName, const std::string & _bindAddress,
                        const std::string & _bindPort, std::size_t _thread_pool_size);
protected:
    TCPConnection * createTCPConnection(boost::asio::io_service & iosrvc);
};

#endif

} } /* namespace */

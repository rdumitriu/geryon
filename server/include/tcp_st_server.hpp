/*
 * TCPServer.h
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCP_ST_SERVER_H_
#define TCP_ST_SERVER_H_

#include <string>

#include <boost/asio.hpp>

#include "tcp_server.hpp"

namespace geryon { namespace server {

///
/// Single threaded acceptor server
///
class SingleThreadTCPServer : public TCPServer {
public:
    SingleThreadTCPServer(const std::string & _srvName, const std::string & _bindAddress,
                          const std::string & _bindPort, TCPProtocol & _proto);

    /// Destructor
    virtual ~SingleThreadTCPServer() {}

    ///Non-copyable
    SingleThreadTCPServer(const SingleThreadTCPServer & copy) = delete;
    ///Non-copyable
    SingleThreadTCPServer & operator = (const SingleThreadTCPServer & other) = delete;

    ///\brief The run loop. You must invoke this for the magic to begin
    virtual void run ();

protected:
    virtual TCPConnectionManager & connectionManager();
private:
    std::shared_ptr<TCPConnectionManager> connMgr;
};

} }  /* namespace */

#endif

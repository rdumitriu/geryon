/*
 * TCPServer.h
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef GERYON_TCP_STA_SERVER_H_
#define GERYON_TCP_STA_SERVER_H_

#include <string>

#include <boost/asio.hpp>

#include "tcp_server.hpp"

namespace geryon { namespace server {

///
/// Single threaded acceptor server.
/// Note: Here, single threaded refers to 'acceptor' ONLY.
///
class SingleThreadAcceptorTCPServer : public TCPServer {
public:
    SingleThreadAcceptorTCPServer(const std::string & _srvName, const std::string & _bindAddress,
                                  const std::string & _bindPort, TCPProtocol & _proto, bool trackConnections);

    /// Destructor
    virtual ~SingleThreadAcceptorTCPServer() {}

    ///Non-copyable
    SingleThreadAcceptorTCPServer(const SingleThreadAcceptorTCPServer & copy) = delete;
    ///Non-copyable
    SingleThreadAcceptorTCPServer & operator = (const SingleThreadAcceptorTCPServer & other) = delete;

    ///\brief The run loop. You must invoke this for the magic to begin
    virtual void run ();

protected:
    virtual TCPConnectionManager & connectionManager();
private:
    std::shared_ptr<TCPConnectionManager> connMgr;
};

} }  /* namespace */

#endif

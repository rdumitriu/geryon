/**
 * \file tcp_mt_server.hpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCP_MTA_SERVER_H_
#define TCP_MTA_SERVER_H_

#include <string>

#include <boost/asio.hpp>

#include "tcp_server.hpp"

namespace geryon { namespace server {

///
/// Multi-threaded acceptor server
/// Note: Here, multi threaded refers to 'acceptor' ONLY.
///
class MultiThreadedAcceptorTCPServer : public TCPServer{
public:
    MultiThreadedAcceptorTCPServer(const std::string & _srvName, const std::string & _bindAddress,
                                   const std::string & _bindPort, TCPProtocol & _proto,
                                   unsigned int _nThreads, bool trackConnections);
    /// Destructor
    virtual ~MultiThreadedAcceptorTCPServer() {}

    ///Non-copyable
    MultiThreadedAcceptorTCPServer(const MultiThreadedAcceptorTCPServer & copy) = delete;
    ///Non-copyable
    MultiThreadedAcceptorTCPServer & operator = (const MultiThreadedAcceptorTCPServer & other) = delete;

    ///\brief The run loop. You must invoke this for the magic to begin
    virtual void run();

protected:
    virtual TCPConnectionManager & connectionManager();
private:
    unsigned int nThreads;
    std::shared_ptr<TCPConnectionManager> connMgr;
};

} }  /* namespace */

#endif /* TCPSERVER_H_ */

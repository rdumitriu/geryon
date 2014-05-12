/**
 * \file tcp_mt_server.hpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCP_MT_SERVER_H_
#define TCP_MT_SERVER_H_

#include <string>

#include <boost/asio.hpp>

#include "tcp_server.hpp"

namespace geryon { namespace server {

///
/// Multi-threaded acceptor server
/// Note: Here, multi threaded refers to 'acceptor' ONLY.
///
class MultiThreadedTCPServer : public TCPServer{
public:
    MultiThreadedTCPServer(const std::string & _srvName, const std::string & _bindAddress,
                           const std::string & _bindPort, TCPProtocol & _proto, unsigned int _nThreads);
    /// Destructor
    virtual ~MultiThreadedTCPServer() {}

    ///Non-copyable
    MultiThreadedTCPServer(const MultiThreadedTCPServer & copy) = delete;
    ///Non-copyable
    MultiThreadedTCPServer & operator = (const MultiThreadedTCPServer & other) = delete;

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

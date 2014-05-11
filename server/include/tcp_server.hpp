/**
 * \file tcp_server.hpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>

#include <boost/asio.hpp>

#include "tcp_protocol.hpp"
#include "tcp_connection_manager.hpp"


namespace geryon { namespace server {

/// The pure TCP server
class TCPServer {
public:
    TCPServer(const std::string & _srvName, const std::string & _bindAddress,
              const std::string & _bindPort, TCPProtocol & _proto);

    /// Destructor
    virtual ~TCPServer();

    ///Non-copyable
    TCPServer (const TCPServer & copy) = delete;
    ///Non-copyable
    TCPServer & operator = (const TCPServer & other) = delete;

    ///\brief The run loop. You must invoke this for the magic to begin
    virtual void run () = 0;

    ///\brief Stops brutally the server
    void stop();

    inline TCPProtocol & protocol() { return proto; }

protected:
    virtual TCPConnectionManager & connectionManager() = 0;

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service iosrvc;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set ssignals;

    ///The next accepted connection socket
    boost::asio::ip::tcp::socket socket;

    std::string serverName;
    std::string bindAddress;
    std::string bindPort;

    TCPProtocol & proto;
private:
    void accept();

    void await_stop_signal();
};

} }  /* namespace */

#endif /* TCPSERVER_H_ */

/*
 * TCPServer.h
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

namespace geryon { namespace server {

class TCPConnection;

/// The pure TCP server
class TCPServer {
public:
    TCPServer(const std::string & _srvName, const std::string & _bindAddress,
              const std::string & _bindPort, std::size_t _thread_pool_size);

    /// Destructor
    virtual ~TCPServer();

    ///Non-copyable
    TCPServer (const TCPServer & copy) = delete;
    ///Non-copyable
    TCPServer & operator = (const TCPServer & other) = delete;

	///the run loop
    void operator ()();

	///stops the server
    void stop();

protected:
	///You must rewrite this to create a connection.
    virtual TCPConnection * createTCPConnection(boost::asio::io_service & iosrvc) = 0;
private:

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service iosrvc;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor;

    ///The next accepted connection
    boost::shared_ptr<TCPConnection> acceptedConnection;

	///accept handler (asynchronous)
	void acceptConnection(const boost::system::error_code& e);

	void prepareNextConnection();

    std::string serverName;
    std::string bindAddress;
    std::string bindPort;

	/// The number of threads that will call io_service::run().
    std::size_t thrpool_sz;

};

} }  /* namespace */

#endif /* TCPSERVER_H_ */

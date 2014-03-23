/**
 * \file tcp_connection.hpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef TCPCONNECTION_HPP_
#define TCPCONNECTION_HPP_

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>


namespace geryon { namespace server {

class TCPProtocolHandler;
/**
 * \brief TCP Connection class.
 *
 * Actually, we handle here the protocol itself, in an asynchronous way for both receives and sends. Close happens when
 * no asynchronous operations were requested anymore.
 */
class TCPConnection :
            public boost::enable_shared_from_this<TCPConnection> {
public:
	///Constructor
	TCPConnection(boost::asio::io_service& io_service,
                  TCPProtocolHandler * const pProtocolHandler);
    ///Destructor
    virtual ~TCPConnection();

    /// Non-copyable
    TCPConnection(const TCPConnection & copy) = delete;
    /// Non-copyable
    TCPConnection & operator = (const TCPConnection & other) = delete;

	/// Call this to start the protocol
    virtual void doCommunication();

	///Need the socket? Here it is...
    boost::asio::ip::tcp::socket& tcpSocket();
private:

	/// Handler for READ
	void readHandler(const boost::system::error_code& e,
				     std::size_t bytes_transferred);

	/// Handler for WRITE
	void writeHandler(const boost::system::error_code& e,
					  std::size_t bytes_transferred);
    /// Handler for CLOSE
    void closeHandler(const boost::system::error_code& e);

	/// Strand (mutex)
    boost::asio::io_service::strand strand;

	/// The socket
    boost::asio::ip::tcp::socket socket;

protected:

	///Schedule another read. We're asynchronous.
	void readMore();
	///Schedule another write. We're asynchronous.
	void writeMore();
    
    ///Gracefully initiate close
    void requestClose();
    
	///Closes the socket (abruptly).
	void close();
    
    ///Async data handler, called after each read ?!?
	virtual void processReadData(std::size_t nBytes) = 0;
    ///Async data writer, called after each write ?!?
	virtual void processWrittenData(std::size_t nBytes) = 0;

private:
    TCPProtocolHandler * protocolHandler;
};

} }  /* namespace */

#endif /* TCPCONNECTION_HPP_ */

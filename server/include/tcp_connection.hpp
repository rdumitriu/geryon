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

#include <deque>
#include <mutex>

#include "tcp_protocol_handler.hpp"
#include "tcp_connection_manager.hpp"

namespace geryon { namespace server {

typedef boost::asio::mutable_buffers_1 asioBuffer;

/**
 * \brief TCP Connection class.
 *
 * The connections represents the 'bridge' between the protocol itself and the server. This is the last time we hear
 * about asio, the rest should not have asio dependencies at all.
 *
 * Reads and writes are asynchronous. Only the protocol handler is allowed to read and write on the connection.
 */
class TCPConnection {
public:
    ///Constructor
    TCPConnection(boost::asio::ip::tcp::socket && _socket,
                  boost::asio::io_service & io_service,
                  TCPConnectionManager & _connectionManager,
                  std::shared_ptr<TCPProtocolHandler> pProtocolHandler);
    ///Destructor
    virtual ~TCPConnection();

    /// Non-copyable
    TCPConnection(const TCPConnection & copy) = delete;
    /// Non-copyable
    TCPConnection & operator = (const TCPConnection & other) = delete;

    /// Call this to start the protocol
    void start();

    ///Closes the socket (abruptly).
    void stop();

    inline boost::asio::ip::tcp::socket & tcpSocket() { return socket; }

    inline boost::asio::io_service::strand & ioStrand() { return strand; }

    inline TCPConnectionManager & connectionManager() { return rConnectionManager; }

    inline std::shared_ptr<TCPProtocolHandler> & protocolHandler() { return pProtocolHandler; }

protected:
    friend class TCPProtocolHandler;
    ///
    /// \brief Schedule a read.
    ///
    /// Calling with invalid buffer will close the connection
    ///
    /// \param readBuffer the buffer where to read into
    /// \return true if schedule suceeded
    ///
    bool scheduleRead(GBufferHandler && readBuffer);

    ///
    /// \brief Schedule a write
    ///
    /// Calling with invalid buffer will close the connection
    ///
    /// \param writeBuffer the buffer to write from.
    /// \return true if write was scheduled
    ///
    bool scheduleWrite(GBufferHandler && writeBuffer);

    ///
    /// \brief Reschedule a write.
    ///
    /// Should only be called internally, never from outside
    ///
    /// \return true if reschedule succeeded.
    ///
    bool rescheduleWrite();


    virtual void scheduleASIORead() = 0;
    virtual void scheduleASIOWrite(bool reschedule = false) = 0;
    virtual void scheduleASIOClose() = 0;

    /// The read buffer
    GBufferHandler readBuffer;

    /// The write buffers (may accumulate!)
    std::deque<GBufferHandler> writeBuffers;

private:
    /// Strand (mutex for asio, if needed)
    boost::asio::io_service::strand strand;

    /// The socket
    boost::asio::ip::tcp::socket socket;

    /// Connection manager
    TCPConnectionManager & rConnectionManager;

    /// The protocol handler.
    std::shared_ptr<TCPProtocolHandler> pProtocolHandler;

    /// Protects the read and write buffers
    std::mutex mutex;
};

} }  /* namespace */

#endif /* TCPCONNECTION_HPP_ */

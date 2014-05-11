/**
 * \file tcp_protocol_handler.hpp
 *
 *  Created on: Aug 19, 2011, reformatted on Mar 23, 2014
 *      Author: rdumitriu
 */
#ifndef TCPPROTOCOLHANDLER_HPP_
#define TCPPROTOCOLHANDLER_HPP_

#include <deque>

#include <boost/asio.hpp>

#include "mem_buf.hpp"
#include "tcp_connection.hpp"

namespace geryon { namespace server {

class TCPConnection;
///
/// \brief The protocol handler
///
/// The protocol handler, a class representing the interaction protocol between the client and the server. At the very
/// primary level it deals with input/output buffers, accepts the reads from connection and writes to it.
///
class TCPProtocolHandler {
public:
    /// \brief The constructor
    explicit TCPProtocolHandler(GMemoryPool & _rMemoryPool) : rMemoryPool(_rMemoryPool), pConnection(0) {}
    /// \brief Destructor
    virtual ~TCPProtocolHandler() {}

    ///Non-copyable
    TCPProtocolHandler(const TCPProtocolHandler& other) = delete;
    ///Non-copyable
    TCPProtocolHandler & operator = (const TCPProtocolHandler &other) = delete;
    
    /// \brief Called just before the protocol is used
    virtual void init(TCPConnection & _rConnection) {
        pConnection = &_rConnection;
    }

    ///
    /// \brief We read the bytes, this is where we process them.
    ///
    /// The current marker in the buffers shows where to start.
    ///
    /// \param currentBuffer the current buffer
    /// \param nBytes the number of bytes needing processing
    /// \return the next op
    ///
    virtual void handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) = 0;
    
    /// \brief Called just before exit time
    virtual void done() {}

protected:
    /// \brief Schedule a read (asynchronously)
    void requestRead(GBufferHandler && readBuffer);

    /// \brief Writes the data on the wire (asynchronously).
    /// \return true if the write succeeded, false otherwise
    void requestWrite(GBufferHandler && writeBuffer);

    /// \brief Communication breakdown (asynchronously).
    void requestClose();


    /// \brief Gets the memory pool pointer
    inline GMemoryPool & getMemoryPool() { return rMemoryPool; }

    /// \brief Returns the read hint. Default implementation returns 0 (no hint)
    virtual std::size_t getReadSizeHint() { return 0; }

    /// \brief Returns the write hint. Default implementation returns 0 (no hint)
    virtual std::size_t getWriteSizeHint() { return 0; }

    /// \brief get connection
    inline TCPConnection & getConnection() { return *(pConnection); }

private:
    /// \brief The memory pool pointer
    GMemoryPool & rMemoryPool;

    /// \brief The connection this handler belongs to
    TCPConnection * pConnection;
};
    
} } /* namespace */

#endif

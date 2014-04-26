/**
 * \file tcp_protocol_handler.hpp
 *
 *  Created on: Aug 19, 2011, reformatted on Mar 23, 2014
 *      Author: rdumitriu
 */
#ifndef TCPPROTOCOLHANDLER_HPP_
#define TCPPROTOCOLHANDLER_HPP_

#include <boost/asio.hpp>

namespace geryon { namespace server {

typedef boost::asio::mutable_buffers_1 asioBuffer;

/**
 * \brief The protocol handler
 *
 * The protocol handler, a class representing the interaction protocol between the client and the server
 */
class TCPProtocolHandler {
public:
    enum Operation { READ, WRITE, CLOSE };
    ///The constructor
    explicit TCPProtocolHandler(bool _clientInitiated = true) : clientInitiated(_clientInitiated) {}
    ///Destructor
    virtual ~TCPProtocolHandler() {}
    
    ///Called just before the protocol is used
    virtual void init() {}
    
    ///True if the protocol starts on the client side, false if server sends first the data
    inline bool isClientInitiated() const { return clientInitiated; }

    ///Called by the connection when we read the bytes
    virtual Operation read(std::size_t nBytes) = 0; //TODO:: implement

    ///The read buffer
    virtual asioBuffer readBuffer() = 0; //TODO:: implement

    ///Called by the connection when we wrote the bytes
    virtual Operation write(std::size_t nBytes) = 0; //TODO:: implement

    ///The write buffer
    virtual asioBuffer writeBuffer() = 0; //TODO:: implement
    
    ///called just before exit time
    virtual void done() {}
private:
    bool clientInitiated;
};
    
} } /* namespace */

#endif

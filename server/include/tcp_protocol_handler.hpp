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

/**
 * \brief The protocol handler
 *
 * The protocol handler, an abstract class representing the interaction protocol between the client and the server
 */
class TCPProtocolHandler {
public:
    ///The constructor
    explicit TCPProtocolHandler(bool _clientInitiated = true) : clientInitiated(_clientInitiated) {}
    ///Destructor
    virtual ~TCPProtocolHandler() {}
    
    ///Called just before the protocol is used
    virtual void init() = 0;
    
    ///True if the protocol starts on the client side, false if server sends first the data
    inline bool isClientInitiated() const { return clientInitiated; }

    virtual void read(std::size_t nBytes) = 0;

    virtual void write(std::size_t nBytes) = 0;

    virtual boost::asio::mutable_buffers_1 readBuffer() = 0;

    virtual boost::asio::mutable_buffers_1 writeBuffer() = 0;
    
    ///called just before exit time
    virtual void done() = 0;
private:
    bool clientInitiated;
};
    
} } /* namespace */

#endif

/**
 * \file tcp_st_server.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <thread>

#include "tcp_connection.hpp"
#include "tcp_protocol_handler.hpp"
#include "tcp_sta_server.hpp"

#include "log.hpp"

namespace geryon { namespace server {

namespace detail {

/* ======================================================================= */
/* C O N N E C T I O N  &  C O N N  M A N A G E R */
/* ======================================================================= */

///Connection, ST
class STTCPConnection : public TCPConnection, public std::enable_shared_from_this<STTCPConnection> {
public:
    STTCPConnection(boost::asio::ip::tcp::socket && _socket, boost::asio::io_service & io_service,
                    TCPConnectionManager & _connectionManager,
                    std::shared_ptr<TCPProtocolHandler> _protocolHandler)
                        : TCPConnection(std::move(_socket), io_service, _connectionManager, _protocolHandler) {
    }

    virtual ~STTCPConnection() {}

private:
    virtual void scheduleASIORead();
    virtual void scheduleASIOWrite();
    virtual void scheduleASIOClose();
};

///Connection manager, ST
class STTCPConnectionManager : public TCPConnectionManager {
public:
    STTCPConnectionManager(TCPProtocol & _proto) : TCPConnectionManager(), proto(_proto) {
        LOG(geryon::util::Log::DEBUG) << "Initialized STTCP conn manager";
    }
    virtual ~STTCPConnectionManager() {
        LOG(geryon::util::Log::DEBUG) << "Cleaning up STTCP conn manager";
    }
protected:
    virtual std::shared_ptr<TCPConnection> create(boost::asio::ip::tcp::socket && socket,
                                                  boost::asio::io_service & ioservice);
private:
    TCPProtocol & proto;
};

/* ======================================================================= */
/* I M P L E M E N T A T I O N */
/* ======================================================================= */

void STTCPConnection::scheduleASIORead() {
    auto self(shared_from_this()); //make sure we live long enough to complete the handler !


    if(commands.empty() || !commands.front()->bufferHandler.isValid() ||
       commands.front()->command != detail::TCPConnectionCommand::READ) {
        LOG(geryon::util::Log::DEBUG) << "Invalid entry in ASIORead, forcing close";
        scheduleASIOClose();
        return;
    }

    GBuffer gbuff = commands.front()->bufferHandler.get();
    if(gbuff.size() == gbuff.marker()) { //full, most probably error, let's close it
        LOG(geryon::util::Log::DEBUG) << "Full buffer provided on ASIO Read, forcing close";
        scheduleASIOClose();
        return;
    }

    asioBuffer asiob(gbuff.buffer() + gbuff.marker(), gbuff.size() - gbuff.marker());

    tcpSocket().async_read_some(asiob,
                                [this, self](boost::system::error_code ec, std::size_t nBytes) {
        try {
            if (!ec) {
                LOG(geryon::util::Log::DEBUG) << "In read handler";
                GBufferHandler localbuff(std::move(commands.front()->bufferHandler));
                protocolHandler()->handleRead(std::move(localbuff), nBytes);
                rescheduleASIO();
            } else if (ec != boost::asio::error::operation_aborted) {
                connectionManager().stop(shared_from_this());
            }
        } catch( ... ) {
            LOG(geryon::util::Log::ERROR) << "Fatal error in processing request. Connection aborted";
            connectionManager().stop(shared_from_this());
        }
      });

}

void STTCPConnection::scheduleASIOWrite() {
    auto self(shared_from_this()); //make sure we live long enough to complete the handler !

    if(commands.empty() || !commands.front()->bufferHandler.isValid() ||
       commands.front()->command != detail::TCPConnectionCommand::WRITE) {
        LOG(geryon::util::Log::DEBUG) << "Invalid entry in ASIOWrite, forcing close";
        scheduleASIOClose();
        return;
    }

    GBuffer gbuff = commands.front()->bufferHandler.get();
    if(gbuff.marker() == 0) { //empty? most probably error, let's close it
        LOG(geryon::util::Log::DEBUG) << "Empty buffer provided for ASIO Write, forcing close";
        scheduleASIOClose();
        return;
    }

    asioBuffer asiob(gbuff.buffer(), gbuff.marker());

    boost::asio::async_write(tcpSocket(), asiob,
                             [this, self](boost::system::error_code ec, std::size_t nBytes) {
        if (!ec) {
            //must remove the first buffer and reschedule write
            LOG(geryon::util::Log::DEBUG) << "In write handler";
            rescheduleASIO();
        } else if (ec != boost::asio::error::operation_aborted) {
            LOG(geryon::util::Log::DEBUG) << "In write handler, error x=" << ec << " nb=" << nBytes;
            connectionManager().stop(shared_from_this());
        }
    });
}

void STTCPConnection::scheduleASIOClose() {
    LOG(geryon::util::Log::DEBUG) << "Requesting close ...";
    auto self(shared_from_this()); //make sure we live long enough to complete the handler !
    boost::asio::async_write(tcpSocket(),
                             boost::asio::buffer(boost::asio::mutable_buffer(NULL, 0)),
                             [this, self](boost::system::error_code ec, std::size_t) {
        LOG(geryon::util::Log::DEBUG) << "Nice shutdown initiated.";
        connectionManager().stop(shared_from_this());
    });
}

std::shared_ptr<TCPConnection> STTCPConnectionManager::create(boost::asio::ip::tcp::socket && socket,
                                                              boost::asio::io_service & ioservice) {
    LOG(geryon::util::Log::DEBUG) << "Creating STTCP connection.";

    return std::make_shared<STTCPConnection>(std::move(socket), ioservice, *this, proto.createHandler());
}

} /* namespace detail */

/* ======================================================================= */
/* T H E  S E R V E R */
/* ======================================================================= */


SingleThreadAcceptorTCPServer::SingleThreadAcceptorTCPServer(const std::string & _srvName,
                                                             const std::string & _bindAddress,
                                                             const std::string & _bindPort, TCPProtocol & _proto)
            : TCPServer(_srvName, _bindAddress, _bindPort, _proto) {
    connMgr = std::make_shared<detail::STTCPConnectionManager>(_proto);
}

void SingleThreadAcceptorTCPServer::run() {
    iosrvc.run();
}


TCPConnectionManager & SingleThreadAcceptorTCPServer::connectionManager() {
    return *(connMgr.get());
}


} } /* namespace */

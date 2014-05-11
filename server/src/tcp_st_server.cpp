/**
 * \file tcp_server.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <thread>

#include "tcp_connection.hpp"
#include "tcp_st_server.hpp"

#include "log.hpp"

namespace geryon { namespace server {

namespace detail {

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
    virtual void scheduleASIOWrite(bool reschedule);
    virtual void scheduleASIOClose();
};

///Connection manager, ST
class STTCPConnectionManager : public TCPConnectionManager {
public:
    STTCPConnectionManager(TCPProtocol & _proto) : TCPConnectionManager(), proto(_proto) {
        LOG(geryon::util::Log::INFO) << "Initialized STTCP conn manager";
    }
    virtual ~STTCPConnectionManager() {
        LOG(geryon::util::Log::INFO) << "Cleaning up STTCP conn manager";
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

    GBuffer gbuff = readBuffer.get();
    if(!gbuff.isValid()) { //invalid, close requested
        LOG(geryon::util::Log::DEBUG) << "Invalid buffer on read, close request";
        scheduleASIOClose();
        return;
    }
    if(gbuff.size() == gbuff.marker()) { //full, most probably error, let's close it
        LOG(geryon::util::Log::DEBUG) << "Full buffer provided on read, close request";
        scheduleASIOClose();
        return;
    }

    asioBuffer asiob(gbuff.buffer() + gbuff.marker(), gbuff.size() - gbuff.marker());

    tcpSocket().async_read_some(asiob,
                                [this, self](boost::system::error_code ec, std::size_t nBytes) {
        if (!ec) {
            LOG(geryon::util::Log::DEBUG) << "In read handler";
            GBufferHandler localbuff(std::move(readBuffer));
            protocolHandler()->handleRead(std::move(localbuff), nBytes);
        } else if (ec != boost::asio::error::operation_aborted) {
            connectionManager().stop(shared_from_this());
        }
      });

}

void STTCPConnection::scheduleASIOWrite(bool reschedule) {
    auto self(shared_from_this()); //make sure we live long enough to complete the handler !
    if(writeBuffers.empty()) { //that's a protocol error !
        LOG(geryon::util::Log::DEBUG) << "Empty buffers at write time (zero)";
        scheduleASIOClose();
        return;
    }
    //if we have already a write in progress, we must not proceed further
    if(!(writeBuffers.size() == 1 || reschedule)) {
        LOG(geryon::util::Log::DEBUG) << "Not first buffer or not rescheduled.";
        return; //continue only if ((first write in a serie of writes) OR (reschedule))
    }

    GBuffer gbuff = writeBuffers.front().get();
    if(!gbuff.isValid()) { //invalid, close requested
        LOG(geryon::util::Log::DEBUG) << "Invalid buffer provided for write. Requesting close";
        scheduleASIOClose();
        return;
    }
    if(gbuff.marker() == 0) { //empty? most probably error, let's close it
        LOG(geryon::util::Log::DEBUG) << "Empty buffer provided for write. Requesting close";
        scheduleASIOClose();
        return;
    }

    asioBuffer asiob(gbuff.buffer(), gbuff.marker());

    boost::asio::async_write(tcpSocket(), asiob,
                             [this, self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
            //must remove the first buffer and reschedule write
            LOG(geryon::util::Log::DEBUG) << "In write handler";
            rescheduleWrite();
        } if (ec != boost::asio::error::operation_aborted) {
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


SingleThreadTCPServer::SingleThreadTCPServer(const std::string & _srvName, const std::string & _bindAddress,
                                             const std::string & _bindPort, TCPProtocol & _proto)
            : TCPServer(_srvName, _bindAddress, _bindPort, _proto) {
    connMgr = std::make_shared<detail::STTCPConnectionManager>(_proto);
}

void SingleThreadTCPServer::run() {
    iosrvc.run();
}

TCPConnectionManager & SingleThreadTCPServer::connectionManager() {
    return *(connMgr.get());
}


} } /* namespace */

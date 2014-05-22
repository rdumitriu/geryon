/**
 * \file tcp_mt_server.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <thread>

#include <boost/bind.hpp>

#include "os_utils.hpp"
#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"
#include "tcp_mta_server.hpp"

#include "log.hpp"

namespace geryon { namespace server {

namespace detail {

/* ======================================================================= */
/* C O N N E C T I O N  &  C O N N  M A N A G E R */
/* ======================================================================= */

///Connection, MT
class MTTCPConnection : public TCPConnection, public std::enable_shared_from_this<MTTCPConnection> {
public:
    MTTCPConnection(boost::asio::ip::tcp::socket && _socket, boost::asio::io_service & io_service,
                    TCPConnectionManager & _connectionManager,
                    std::shared_ptr<TCPProtocolHandler> _protocolHandler)
                        : TCPConnection(std::move(_socket), io_service, _connectionManager, _protocolHandler),
                          strand(io_service) {
    }

    virtual ~MTTCPConnection() {}

    inline boost::asio::io_service::strand & ioStrand() { return strand; }
private:
    virtual void scheduleASIORead();
    virtual void scheduleASIOWrite();
    virtual void scheduleASIOClose();

    /// Strand (mutex for asio, needed)
    boost::asio::io_service::strand strand;

};

///Connection manager, MT
class MTTCPConnectionManager : public TCPConnectionManager {
public:
    MTTCPConnectionManager(TCPProtocol & _proto) : TCPConnectionManager(), proto(_proto) {
        LOG(geryon::util::Log::DEBUG) << "Initialized MTTCP conn manager";
    }
    virtual ~MTTCPConnectionManager() {
        LOG(geryon::util::Log::DEBUG) << "Cleaning up MTTCP conn manager";
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

void MTTCPConnection::scheduleASIORead() {
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
                                ioStrand().wrap([this, self](boost::system::error_code ec, std::size_t nBytes) {
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
      }));

}

void MTTCPConnection::scheduleASIOWrite() {
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
                             ioStrand().wrap([this, self](boost::system::error_code ec, std::size_t nBytes) {
        if (!ec) {
            //must remove the first buffer and reschedule write
            LOG(geryon::util::Log::DEBUG) << "In write handler";
            rescheduleASIO();
        } else if (ec != boost::asio::error::operation_aborted) {
            LOG(geryon::util::Log::DEBUG) << "In write handler, error x=" << ec << " nb=" << nBytes;
            connectionManager().stop(shared_from_this());
        }
    }));
}

void MTTCPConnection::scheduleASIOClose() {
    LOG(geryon::util::Log::DEBUG) << "Requesting close ...";
    auto self(shared_from_this()); //make sure we live long enough to complete the handler !
    boost::asio::async_write(tcpSocket(),
                             boost::asio::buffer(boost::asio::mutable_buffer(NULL, 0)),
                             ioStrand().wrap([this, self](boost::system::error_code ec, std::size_t) {
        LOG(geryon::util::Log::DEBUG) << "Nice shutdown initiated.";
        connectionManager().stop(shared_from_this());
    }));
}

std::shared_ptr<TCPConnection> MTTCPConnectionManager::create(boost::asio::ip::tcp::socket && socket,
                                                              boost::asio::io_service & ioservice) {
    LOG(geryon::util::Log::DEBUG) << "Creating MTTCP connection.";

    return std::make_shared<MTTCPConnection>(std::move(socket), ioservice, *this, proto.createHandler());
}

} /* namespace detail */

/* ======================================================================= */
/* T H E  S E R V E R */
/* ======================================================================= */

MultiThreadedAcceptorTCPServer::MultiThreadedAcceptorTCPServer(const std::string & _srvName,
                                                               const std::string & _bindAddress,
                                                               const std::string & _bindPort,
                                                               TCPProtocol & _proto, unsigned int _nThreads)
                        : TCPServer(_srvName, _bindAddress, _bindPort, _proto), nThreads(_nThreads) {
    connMgr = std::make_shared<detail::MTTCPConnectionManager>(_proto);
}


void MultiThreadedAcceptorTCPServer::run() {
    std::shared_ptr<std::thread> * threads = NULL;
    try {
        // Create the thread pool
        LOG(geryon::util::Log::DEBUG) << "Creating thread pool, size = " << nThreads;
        threads = new std::shared_ptr<std::thread>[nThreads];
        for (unsigned int i = 0; i < nThreads; ++i) {
            threads[i] = std::shared_ptr<std::thread>(new std::thread([this]() {
                iosrvc.run();
            }));
        }

        // join all
        for (unsigned int i = 0; i < nThreads; ++i) {
            threads[i]->join();
        }
        delete [] threads;
        LOG(geryon::util::Log::DEBUG) << "Deallocated thread pool, size = " << nThreads;
    } catch(...) {
        LOG(geryon::util::Log::FATAL) << "Server >>" << serverName << ": Forced exit for the thread pool, getting out.";
        delete [] threads;
        throw;
    }
}


TCPConnectionManager & MultiThreadedAcceptorTCPServer::connectionManager() {
    return *(connMgr.get());
}


} } /* namespace */



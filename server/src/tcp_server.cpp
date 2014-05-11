/**
 * \file tcp_server.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#include <thread>

#include <boost/bind.hpp>

#include "os_utils.hpp"
#include "tcp_server.hpp"
#include "tcp_connection.hpp"

#include "log.hpp"

namespace geryon { namespace server {

TCPServer::TCPServer(const std::string & _srvName, const std::string & _bindAddress,
                     const std::string & _bindPort, TCPProtocol & _proto)
                : acceptor(iosrvc),
                  ssignals(iosrvc),
                  socket(iosrvc),
                  serverName(_srvName),
                  bindAddress(_bindAddress),
                  bindPort(_bindPort),
                  proto(_proto) {

    ssignals.add(SIGINT);
    ssignals.add(SIGTERM);
  #if defined(SIGQUIT)
    ssignals.add(SIGQUIT);
  #endif
    await_stop_signal(); //register wait handler

    boost::asio::ip::tcp::resolver resolver(iosrvc);
    boost::asio::ip::tcp::resolver::query query(bindAddress, bindPort);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor.open(endpoint.protocol());
#ifndef G_HAS_WIN
	//use SO_REUSEADDR on Unix
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
#endif
    acceptor.bind(endpoint);
    acceptor.listen();
    accept();
    LOG(geryon::util::Log::INFO) << "Server >>" << serverName << "<< is listening on " << bindAddress << " port:" << bindPort;
}

void TCPServer::accept() {
  acceptor.async_accept(
            socket,
            [this](boost::system::error_code ec) {
                if (!acceptor.is_open()) {
                  return;
                }

                //wrap it
                if (!ec) {
                    LOG(geryon::util::Log::DEBUG) << "Executing connMgr::start";
                    connectionManager().start(std::move(socket), iosrvc);
                } else {
                    LOG(geryon::util::Log::ERROR) << "Server >>" << serverName
                                                  << ": Problem executing accept(), error code:" << ec;
                }
                accept();
    });
}

void TCPServer::await_stop_signal() {
    ssignals.async_wait(
        [this](boost::system::error_code /*ec*/, int /*signo*/) {
            acceptor.close();
            //clean all the connections
            connectionManager().stopAll();
            stop();
    });
}

void TCPServer::stop() {
    iosrvc.stop();
}

TCPServer::~TCPServer() {
    LOG(geryon::util::Log::INFO) << "Server >>" << serverName << "<< bound on " << bindAddress
                                 << ", port:" << bindPort << " stopped.";
}


} } /* namespace */

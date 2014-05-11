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

//TCPServer::TCPServer(const std::string & _srvName, const std::string & _bindAddress,
//                     const std::string & _bindPort)
//                : acceptor(iosrvc),
//                  ssignals(iosrvc),
//                  acceptedConnection(),
//                  serverName(_srvName),
//                  bindAddress(_bindAddress),
//                  bindPort(_bindPort) {

//    ssignals.add(SIGINT);
//    ssignals.add(SIGTERM);
//  #if defined(SIGQUIT)
//    ssignals.add(SIGQUIT);
//  #endif
//    await_stop_signal(); //register wait handler

//    boost::asio::ip::tcp::resolver resolver(iosrvc);
//    boost::asio::ip::tcp::resolver::query query(bindAddress, bindPort);
//	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
//    acceptor.open(endpoint.protocol());
//#ifndef G_HAS_WIN
//	//use SO_REUSEADDR on Unix
//    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
//#endif
//    acceptor.bind(endpoint);
//    acceptor.listen();
//    LOG(geryon::util::Log::INFO) << "Server >>" << serverName << "<< is listening on " << bindAddress << " port:" << bindPort;
//}


//void TCPServer::run() {
//    boost::shared_ptr<std::thread> * threads = NULL;
//	try {
//		//prepare first accept
//		prepareNextConnection();
//		// Create the thread pool
//        LOG(geryon::util::Log::DEBUG) << "Creating thread pool, size = " << thrpool_sz;
//        threads = new boost::shared_ptr<std::thread>[thrpool_sz];
//        for (std::size_t i = 0; i < thrpool_sz; ++i) {
//            threads[i] = boost::shared_ptr<std::thread>(new std::thread(boost::bind(&boost::asio::io_service::run, &iosrvc)));
//		}

//		// join all
//        for (std::size_t i = 0; i < thrpool_sz; ++i) {
//			threads[i]->join();
//		}
//		delete [] threads;
//        LOG(geryon::util::Log::DEBUG) << "Deallocated thread pool, size = " << thrpool_sz;
//	} catch(...) {
//        LOG(geryon::util::Log::FATAL) << "Server >>" << serverName << ": Forced exit for the thread pool, getting out.";
//		delete [] threads;
//		throw;
//	}
//}

//void TCPServer::prepareNextConnection() {
//    acceptedConnection.reset(createTCPConnection(iosrvc));
//    acceptor.async_accept(acceptedConnection->tcpSocket(),
//							boost::bind(&TCPServer::acceptConnection,
//										this,
//										boost::asio::placeholders::error));
//}

//void TCPServer::acceptConnection(const boost::system::error_code& e) {
//	if (!e) {
//        acceptedConnection->doCommunication(); //start the madness here!
//		prepareNextConnection();
//	} else {
//        LOG(geryon::util::Log::ERROR) << "Server >>" << serverName << ": Problem executing acceptConnection(), error code:"
//                                      << e.value() << " [" << e.message() << "]";
//	}
//}

//void TCPServer::accept() {
//  acceptor.async_accept(
//            socket,
//            [this](boost::system::error_code ec) {
//                if (!acceptor.is_open()) {
//                  return;
//                }

//                //wrap it
//                if (!ec) {
//                  connectionManager().start(std::move(socket), iosrvc));
//                } else {
//                    LOG(geryon::util::Log::ERROR) << "Server >>" << serverName << ": Problem executing accept(), error code:"
//                                                  << e.value() << " [" << e.message() << "]";
//                }
//                accept();
//    });
//}

//void TCPServer::await_stop_signal() {
//    ssignals_.async_wait(
//        [this](boost::system::error_code /*ec*/, int /*signo*/) {
//            acceptor.close();
//            //clean nicely the connections
//            connectionManager().stopAll();
//            stop();
//    });
//}

//void TCPServer::stop() {
//    iosrvc.stop();
//}

//TCPServer::~TCPServer() {
//    LOG(geryon::util::Log::INFO) << "Server >>" << serverName << "<< bound on " << bindAddress
//                                 << ", port:" << bindPort << " stopped.";
//}


} } /* namespace */

/**
 * \file tcp_connection_manager.hpp
 *
 *  Created on: Aug 13, 2011
 *      Author: rdumitriu
 */

#ifndef GERYON_TCPCONNECTIONMANAGER_HPP_
#define GERYON_TCPCONNECTIONMANAGER_HPP_

#include <set>
#include <mutex>

#include <boost/asio.hpp>


namespace geryon { namespace server {

class TCPConnection;

/// Manages all the connections.
class TCPConnectionManager {
public:

    TCPConnectionManager(bool _trackConnections) : trackConnections(_trackConnections) {}
    virtual ~TCPConnectionManager() {}

    ///Non-copyable
    TCPConnectionManager(const TCPConnectionManager&) = delete;
    ///Non-copyable
    TCPConnectionManager& operator=(const TCPConnectionManager&) = delete;

    /// \brief Create the connection and start it.
    std::shared_ptr<TCPConnection> start(boost::asio::ip::tcp::socket && socket,
                                         boost::asio::io_service & ioservice);

    /// \brief End of life-cycle.
    void stop(std::shared_ptr<TCPConnection> c);

    /// \brief Stop all connections.
    void stopAll();

protected:
    virtual std::shared_ptr<TCPConnection> create(boost::asio::ip::tcp::socket && socket,
                                                  boost::asio::io_service & ioservice) = 0;

private:
  /// The underlying connections.
  std::set<std::shared_ptr<TCPConnection>> connections;
  bool trackConnections;
  std::mutex mutex;
};

} } /*namespace */

#endif

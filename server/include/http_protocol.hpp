///
/// \file http_protocol.hpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_HTTPPROTOCOL_HPP_
#define GERYON_HTTPPROTOCOL_HPP_

#include <memory>

#include "tcp_protocol.hpp"
#include "http_executor.hpp"

namespace geryon { namespace server {


class HttpProtocol : public TCPProtocol {
public:
    HttpProtocol(HttpExecutor & _executor, std::size_t _maxRequestLenght = 1024 * 1024 * 10)
                    : TCPProtocol("http"),
                      executor(_executor),
                      maxRequestLenght(_maxRequestLenght) {}
    virtual ~HttpProtocol() {}

    virtual std::shared_ptr<TCPProtocolHandler> createHandler();

private:
    HttpExecutor & executor;
    std::size_t maxRequestLenght;
};

} }

#endif

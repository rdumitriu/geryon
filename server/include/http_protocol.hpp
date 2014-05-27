///
/// \file http_protocol.hpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#ifndef HTTPPROTOCOL_HPP_
#define HTTPPROTOCOL_HPP_

#include <memory>

#include "tcp_protocol.hpp"
#include "http_executor.hpp"

namespace geryon { namespace server {


class HttpProtocol : public TCPProtocol {
public:
    HttpProtocol(HttpExecutor * const _executor, std::size_t _maxRequestLenght = 1024 * 1024 * 10)
                    : TCPProtocol("http"),
                      pExecutor(_executor),
                      maxRequestLenght(_maxRequestLenght) {}
    virtual ~HttpProtocol() {}

    virtual std::shared_ptr<TCPProtocolHandler> createHandler();

private:
    HttpExecutor * pExecutor;
    std::size_t maxRequestLenght;
};

} }

#endif

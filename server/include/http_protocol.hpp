///
/// \file http_protocol.hpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#ifndef HTTPPROTOCOL_HPP_
#define HTTPPROTOCOL_HPP_

#include <memory>

#include "tcp_protocol.hpp"

namespace geryon { namespace server {

class HttpProtocol : public TCPProtocol {
public:
    HttpProtocol() : TCPProtocol("http") {}
    virtual ~HttpProtocol() {}

    virtual std::shared_ptr<TCPProtocolHandler> createHandler();
};

} }

#endif

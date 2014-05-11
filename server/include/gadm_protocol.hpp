///
/// \file gadm_protocol.hpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#ifndef GADMPROTOCOL_HPP_
#define GADMPROTOCOL_HPP_

#include <memory>

#include "tcp_protocol.hpp"

namespace geryon { namespace server {

class GAdmProtocol : public TCPProtocol {
public:
    GAdmProtocol() : TCPProtocol("gadm") {}
    virtual ~GAdmProtocol() {}

    virtual std::shared_ptr<TCPProtocolHandler> createHandler();
};

} }

#endif

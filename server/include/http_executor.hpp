///
/// \file http_executor.hpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#ifndef HTTPEXECUTOR_HPP_
#define HTTPEXECUTOR_HPP_

#include "http_server_types.hpp"

namespace geryon { namespace server {

class HttpExecutor {
public:
    HttpExecutor() {}
    virtual ~HttpExecutor() {}

    virtual void execute(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException) = 0;
};

} }

#endif

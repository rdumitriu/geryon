///
/// \file http_mt_executor.hpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#ifndef HTTPMULTITHREADEXECUTOR_HPP_
#define HTTPMULTITHREADEXECUTOR_HPP_

#include "http_executor.hpp"

namespace geryon { namespace server {

class HttpMultiThreadExecutor : public HttpExecutor {
public:
    HttpMultiThreadExecutor() {}
    virtual ~HttpMultiThreadExecutor() {}

    virtual void execute(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException) = 0;
};

} }

#endif

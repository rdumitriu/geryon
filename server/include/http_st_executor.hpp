///
/// \file http_st_executor.hpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_HTTPSINGLETHREADEXECUTOR_HPP_
#define GERYON_HTTPSINGLETHREADEXECUTOR_HPP_

#include "http_executor.hpp"

namespace geryon { namespace server {

class HttpSingleThreadExecutor : public HttpExecutor {
public:
    HttpSingleThreadExecutor() : HttpExecutor() {}
    virtual ~HttpSingleThreadExecutor() {}

protected:
    virtual void executeInternal(std::shared_ptr<ServerApplication> papp,
                                 HttpServerRequest & request,
                                 HttpServerResponse & response);
};

} }

#endif

///
/// \file http_executor.hpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_HTTPEXECUTOR_HPP_
#define GERYON_HTTPEXECUTOR_HPP_

#include "http_server_types.hpp"
#include "server_application.hpp"

namespace geryon { namespace server {

class HttpExecutor {
public:
    HttpExecutor() {}
    virtual ~HttpExecutor() {}

    void execute(HttpServerRequest & request, HttpServerResponse & response);

protected:
    std::shared_ptr<ServerApplication> getDispatchTarget(HttpServerRequest & request);

    virtual void executeInternal(std::shared_ptr<ServerApplication> papp,
                                 HttpServerRequest & request,
                                 HttpServerResponse & response) = 0;
private:
    std::string extractPath(const std::string & path);
    bool verifyDispatchTarget(std::shared_ptr<ServerApplication> papp,
                              HttpServerRequest & request, HttpServerResponse & response);
};

} }

#endif

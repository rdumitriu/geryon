///
/// \file http_executor.cpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#include "http_executor.hpp"
#include "server_global_structs.hpp"
#include "log.hpp"

namespace geryon { namespace server {

void HttpExecutor::execute(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException) {
    std::shared_ptr<ServerApplication> papp = getDispatchTarget(request);
    if(verifyDispatchTarget(papp, request, response)) {
        LOG(geryon::util::Log::DEBUG) << "Application: " << papp->getPath() << " is matching URI path:" << request.getURIPath();
        this->executeInternal(papp, request, response);
    }
}

bool HttpExecutor::verifyDispatchTarget(std::shared_ptr<ServerApplication> papp,
                                        HttpServerRequest & request, HttpServerResponse & response) {
    std::string msg;
    bool err = false;
    if(!papp.get()) {
        msg = "No application found matching URI path: " + request.getURIPath();
        err = true;
    } else if(papp->getStatus() != ApplicationModule::STARTED) {
        msg = "Application found matching URI path: " + request.getURIPath() + " but it is not started!";
        err = true;
    }
    if(err) {
        LOG(geryon::util::Log::DEBUG) << msg;
        response.setStatus(HttpServerResponse::SC_NOT_FOUND);
        response.setContentType(HttpResponse::CT_TEXT);
        response.getOutputStream() << msg;
        response.flush();
        response.close();
    }
    return !err;
}

std::shared_ptr<ServerApplication> HttpExecutor::getDispatchTarget(HttpServerRequest & request) {
    std::string pathSegment = extractPath(request.getURIPath());
    LOG(geryon::util::Log::DEBUG) << "Searching for apps mapped on: " << pathSegment;

    std::shared_ptr<ServerApplication> ret = ServerGlobalStructs::getApplication(pathSegment);
    if(!ret.get()) {
        //if not found, fallback to default app
        ret = ServerGlobalStructs::getApplication("/");
    }
    return ret;
}

std::string HttpExecutor::extractPath(const std::string & path) {
    if(path.length() <= 1) {
        return "/";
    }
    std::size_t pos = path.find('/', 1);
    if(pos == std::string::npos) {
        return path + "/"; //does not end with a slash, append it or full app req
    }
    return path.substr(0, pos+1);
}

} }


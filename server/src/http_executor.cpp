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
    if(!papp.get()) {
        LOG(geryon::util::Log::DEBUG) << "No application found matching URI path: " << request.getURIPath();
        response.setStatus(HttpServerResponse::SC_NOT_FOUND);
        response.close();
    } else {
        LOG(geryon::util::Log::DEBUG) << "Application: " << papp->getPath() << " is matching URI path:" << request.getURIPath();
        this->executeInternal(papp, request, response);
    }
}

std::shared_ptr<ServerApplication> HttpExecutor::getDispatchTarget(HttpServerRequest & request) {
    std::string pathSegment = extractPath(request.getURIPath());
    LOG(geryon::util::Log::DEBUG) << "Searching for apps mapped on: " << pathSegment;

    return ServerGlobalStucts::getApplication(pathSegment);
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

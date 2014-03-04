/*
 * servlet.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: rdumitriu at gmail dot com
 */
#include "servlet.hpp"
#include "log.hpp"

namespace geryon {

void Servlet::execute(HttpRequest * const request, HttpResponse * const reply) {
    LOG(geryon::util::Log::DEBUG) << "Executing servlet for path :" << request->getURIPath();
    switch(request->getMethodCode()) {
        case HttpRequest::GET:
            doGet(request, reply);
            break;
        case HttpRequest::POST:
            doPost(request, reply);
            break;
        case HttpRequest::HEAD:
            doHead(request, reply);
            break;
        case HttpRequest::PUT:
            doPut(request, reply);
            break;
        case HttpRequest::DELETE:
            doDelete(request, reply);
            break;
        case HttpRequest::TRACE:
            doTrace(request, reply);
            break;
        case HttpRequest::OPTIONS:
            doOptions(request, reply);
            break;
        default:
            reply->setStatus(HttpResponse::HttpStatusCode::SC_METHOD_NOT_ALLOWED);
            reply->getOutputStream().flush();
            break;
    }
}

}

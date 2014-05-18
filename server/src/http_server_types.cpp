/**
 * \file http_server_types.cpp
 *
 *  Created on: May 16, 2014
 *      Author: rdumitriu at gmail dot com
 */

#include "http_server_types.hpp"

namespace geryon { namespace server {

void HttpServerRequest::addHeader(const std::string & hdrName, const std::string & hdrValue) {
    std::map<std::string, HttpHeader>::iterator p = headers.find(hdrName);
    if(p == headers.end()) {
        geryon::HttpHeader hdr;
        hdr.name = hdrName;
        hdr.values.push_back(hdrValue);
        headers.insert(make_pair(hdrName, std::move(hdr)));
    } else {
        p->second.values.push_back(hdrValue);
    }
}

void HttpServerRequest::addParameter(const std::string & prmName, const std::string & prmValue) {
    std::map<std::string, std::vector<std::string>>::iterator p = parameters.find(prmName);
    if(p == parameters.end()) {
        std::vector<std::string> vals;
        vals.push_back(prmValue);
        parameters.insert(make_pair(prmName, std::move(vals)));
    } else {
        p->second.push_back(prmValue);
    }
}

void HttpServerRequest::addPart(std::shared_ptr<HttpRequestPart> ptr) {
    parts.push_back(ptr);
}

} }



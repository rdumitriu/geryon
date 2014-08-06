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

std::istream & HttpServerRequest::getInputStream() {
    //NOTE: from the practical point of view, the stream will never!ever start at position 0. So it is safe
    //to assume that zero is a magical value here (signals that the value is not set).
    //LOG(geryon::util::Log::DEBUG) << "Getting the INPUT stream [" << startStreamIndex << "," << endStreamIndex << "]";
    if(0 == endStreamIndex) { //yes, keep it explicit !
        for(unsigned int i = 0; i < buffers.size(); ++i) {
            endStreamIndex += buffers.at(i).get().marker();
        }
        //LOG(geryon::util::Log::DEBUG) << "Setting the INPUT stream [" << startStreamIndex << "," << endStreamIndex << "]";
        buff.setup(startStreamIndex, endStreamIndex);
        stream.rdbuf(&buff); //force update
    }
    return stream;
}

void HttpServerResponse::sendHeaders() {
    if(responseCommitted()) {
        std::string msg = "Response already committed";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
    }
    committed = true;

    //1: status
    stream << getHttpStatusMessage(status);
    //2: content length
    if(contentLength) {
        stream << "Content-Length: " << std::to_string(contentLength) << "\r\n";
    }
    //3: content type
    if(contentType != "") {
        stream << "Content-Type: " << contentType << "\r\n";
    } else {
        stream << "Content-Type: text/plain\r\n";
    }
    //4: the rest of the headers
    for(std::map<std::string, HttpHeader>::iterator i = headers.begin(); i != headers.end(); ++i) {
        for(std::vector<std::string>::iterator j = i->second.values.begin(); j != i->second.values.end(); ++j) {
            stream << i->second.name << ": " << *j << "\r\n";
        }
    }
    stream << "\r\n";
}

} }



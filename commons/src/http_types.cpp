/*
 * http_types.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: rdumitriu at gmail dot com
 */

#include "http_types.hpp"

namespace geryon {

    namespace detail {

        const char * const MSG_GENERIC_HTTP_ERROR = "HTTP unspecified error.";
        const char * const MSG_HEADERS_ALREADY_SENT = "Headers were already sent down the wire. Cannot modify the response.";


        const char * getHttpExceptionCodeMsg(HttpException::HttpExceptionCode code) {
            switch(code) {
                case HttpException::HEADERS_ALREADY_SENT: return MSG_HEADERS_ALREADY_SENT;
                case HttpException::GENERIC_HTTP_ERROR:
                break;
            }
            return MSG_GENERIC_HTTP_ERROR;
        }

    }

HttpException::HttpException(HttpExceptionCode code, const std::string & msg)
    : std::runtime_error(std::move(std::string(detail::getHttpExceptionCodeMsg(code)) + " Error:" + msg)) {
}

HttpException::HttpException(HttpExceptionCode code)
    : std::runtime_error(std::move(std::string(detail::getHttpExceptionCodeMsg(code)))) {
}

/* ====================================================================
 * HttpMessage implementation
 * ================================================================== */


const std::string HttpMessage::getHeaderValue(const std::string & hdrName) const {
    auto p = headers.find(hdrName);
    if(p != headers.end()) {
        if(p->second.values.size() > 0) {
            return p->second.values[0];
        }
    }
    return "";
}

const std::vector<std::string> HttpMessage::getHeaderValues(const std::string & hdrName) const {
    std::vector<std::string> ret;
    auto p = headers.find(hdrName);
    if(p != headers.end()) {
        return p->second.values;
    }
    return ret;
}

void HttpMessage::clear() {
    contentLength = 0UL;
    contentType.clear();
    headers.clear();
    attributes.clear();
}

bool HttpMessage::getCookie(const std::string & cookieName, HttpCookie & cookie) {
    return false;
}

}

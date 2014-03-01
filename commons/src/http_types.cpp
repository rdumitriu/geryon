/*
 * http_types.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: rdumitriu at gmail dot com
 */

#include "http_types.hpp"
#include "string_utils.hpp"
#include "log.hpp"

namespace geryon {

    namespace detail {

        const char * const MSG_GENERIC_HTTP_ERROR = "HTTP unspecified error.";
        const char * const MSG_HEADERS_ALREADY_SENT = "Headers were already sent down the wire. Cannot modify the response.";
        const char * const MSG_COOKIE_MALFORMED = "Cookie is malformed. Please check expires and max-age attributes.";


        const char * getHttpExceptionCodeMsg(HttpException::HttpExceptionCode code) {
            switch(code) {
                case HttpException::HEADERS_ALREADY_SENT: return MSG_HEADERS_ALREADY_SENT;
                case HttpException::COOKIE_MALFORMED: return MSG_COOKIE_MALFORMED;
                case HttpException::GENERIC_HTTP_ERROR:
                break;
            }
            return MSG_GENERIC_HTTP_ERROR;
        }

        const char * const SC_CONTINUE_HDR = "HTTP/1.1 100 Continue\r\n";
        const char * const SC_SWITCHING_PROTOCOLS_HDR = "HTTP/1.1 101 Switching Protocols\r\n";
        const char * const SC_OK_HDR = "HTTP/1.1 200 OK\r\n";
        const char * const SC_CREATED_HDR = "HTTP/1.1 201 Created\r\n";
        const char * const SC_ACCEPTED_HDR = "HTTP/1.1 202 Accepted\r\n";
        const char * const SC_NON_AUTORITATIVE_INFO_HDR = "HTTP/1.1 203 Non-Authoritative Info\r\n";
        const char * const SC_NO_CONTENT_HDR = "HTTP/1.1 204 No Content\r\n";
        const char * const SC_RESET_CONTENT_HDR = "HTTP/1.1 205 Reset Content\r\n";
        const char * const SC_PARTIAL_CONTENT_HDR = "HTTP/1.1 206 Partial Content\r\n";
        const char * const SC_MULTIPLE_CHOICES_HDR = "HTTP/1.1 300 Multiple Choices\r\n";
        const char * const SC_MOVED_PERMANENTLY_HDR = "HTTP/1.1 301 Moved Permanently\r\n";
        const char * const SC_MOVED_TEMPORARILY_HDR = "HTTP/1.1 302 Moved Temporarily\r\n";
        const char * const SC_SEE_OTHER_HDR = "HTTP/1.1 303 See Other\r\n";
        const char * const SC_NOT_MODIFIED_HDR = "HTTP/1.1 304 Not Modified\r\n";
        const char * const SC_USE_PROXY_HDR = "HTTP/1.1 305 Use Proxy\r\n";
        const char * const SC_BAD_REQUEST_HDR = "HTTP/1.1 400 Bad Request\r\n";
        const char * const SC_UNAUTHORIZED_HDR = "HTTP/1.1 401 Unauthorized\r\n";
        const char * const SC_PAYMENT_REQUIRED_HDR = "HTTP/1.1 402 Payment Required\r\n";
        const char * const SC_FORBIDDEN_HDR = "HTTP/1.1 403 Forbidden\r\n";
        const char * const SC_NOT_FOUND_HDR = "HTTP/1.1 404 Not Found\r\n";
        const char * const SC_METHOD_NOT_ALLOWED_HDR = "HTTP/1.1 405 Method Not Allowed\r\n";
        const char * const SC_NOT_ACCEPTABLE_HDR = "HTTP/1.1 406 Not Acceptable\r\n";
        const char * const SC_PROXY_AUTHENTICATION_REQUIRED_HDR = "HTTP/1.1 407 Proxy Authentication Required\r\n";
        const char * const SC_REQUEST_TIMEOUT_HDR = "HTTP/1.1 408 Request Timeout\r\n";
        const char * const SC_CONFLICT_HDR = "HTTP/1.1 409 Conflict\r\n";
        const char * const SC_GONE_HDR = "HTTP/1.1 410 Gone\r\n";
        const char * const SC_LENGTH_REQUIRED_HDR = "HTTP/1.1 411 Length Required\r\n";
        const char * const SC_PRECONDITION_FAILED_HDR = "HTTP/1.1 412 Precondition Failed\r\n";
        const char * const SC_REQUEST_ENTITY_TOO_LARGE_HDR = "HTTP/1.1 413 Request Entity Too Large\r\n";
        const char * const SC_REQUEST_URI_TOO_LONG_HDR = "HTTP/1.1 414 Request-URI Too Long\r\n";
        const char * const SC_UNSUPPORTED_MEDIA_TYPE_HDR = "HTTP/1.1 415 Unsupported Media Type\r\n";
        const char * const SC_REQUESTED_RANGE_NOT_SATISFIABLE_HDR = "HTTP/1.1 416 Requested Range Not Satisfiable\r\n";
        const char * const SC_EXPECTATION_FAILED_HDR = "HTTP/1.1 417 Expectation Failed\r\n";
        const char * const SC_INTERNAL_SERVER_ERROR_HDR = "HTTP/1.1 500 Internal Server Error\r\n";
        const char * const SC_NOT_IMPLEMENTED_HDR = "HTTP/1.1 501 Not Implemented\r\n";
        const char * const SC_BAD_GATEWAY_HDR = "HTTP/1.1 502 Bad Gateway\r\n";
        const char * const SC_SERVICE_UNAVAILABLE_HDR = "HTTP/1.1 503 Service Unavailable\r\n";
        const char * const SC_GATEWAY_TIMEOUT_HDR = "HTTP/1.1 504 Gateway Timeout\r\n";
        const char * const SC_HTTP_VERSION_NOT_SUPPORTED_HDR = "HTTP/1.1 505 HTTP Version Not Supported\r\n";

        const char * getHeaderStatusMessage(HttpResponse::HttpStatusCode status) {
            switch(status) {
                case HttpResponse::SC_CONTINUE: { return SC_CONTINUE_HDR;}
                case HttpResponse::SC_SWITCHING_PROTOCOLS: { return SC_SWITCHING_PROTOCOLS_HDR;}
                case HttpResponse::SC_OK: { return SC_OK_HDR;}
                case HttpResponse::SC_CREATED: { return SC_CREATED_HDR;}
                case HttpResponse::SC_ACCEPTED: { return SC_ACCEPTED_HDR;}
                case HttpResponse::SC_NON_AUTORITATIVE_INFO: { return SC_NON_AUTORITATIVE_INFO_HDR;}
                case HttpResponse::SC_NO_CONTENT: { return SC_NO_CONTENT_HDR;}
                case HttpResponse::SC_RESET_CONTENT: { return SC_RESET_CONTENT_HDR;}
                case HttpResponse::SC_PARTIAL_CONTENT: { return SC_PARTIAL_CONTENT_HDR;}
                case HttpResponse::SC_MULTIPLE_CHOICES: { return SC_MULTIPLE_CHOICES_HDR;}
                case HttpResponse::SC_MOVED_PERMANENTLY: { return SC_MOVED_PERMANENTLY_HDR;}
                case HttpResponse::SC_MOVED_TEMPORARILY: { return SC_MOVED_TEMPORARILY_HDR;}
                case HttpResponse::SC_SEE_OTHER: { return SC_SEE_OTHER_HDR;}
                case HttpResponse::SC_NOT_MODIFIED: { return SC_NOT_MODIFIED_HDR;}
                case HttpResponse::SC_USE_PROXY: { return SC_USE_PROXY_HDR;}
                case HttpResponse::SC_BAD_REQUEST: { return SC_BAD_REQUEST_HDR;}
                case HttpResponse::SC_UNAUTHORIZED: { return SC_UNAUTHORIZED_HDR;}
                case HttpResponse::SC_PAYMENT_REQUIRED: { return SC_PAYMENT_REQUIRED_HDR;}
                case HttpResponse::SC_FORBIDDEN: { return SC_FORBIDDEN_HDR;}
                case HttpResponse::SC_NOT_FOUND: { return SC_NOT_FOUND_HDR;}
                case HttpResponse::SC_METHOD_NOT_ALLOWED: { return SC_METHOD_NOT_ALLOWED_HDR;}
                case HttpResponse::SC_NOT_ACCEPTABLE: { return SC_NOT_ACCEPTABLE_HDR;}
                case HttpResponse::SC_PROXY_AUTHENTICATION_REQUIRED: { return SC_PROXY_AUTHENTICATION_REQUIRED_HDR;}
                case HttpResponse::SC_REQUEST_TIMEOUT: { return SC_REQUEST_TIMEOUT_HDR;}
                case HttpResponse::SC_CONFLICT: { return SC_CONFLICT_HDR;}
                case HttpResponse::SC_GONE: { return SC_GONE_HDR;}
                case HttpResponse::SC_LENGTH_REQUIRED: { return SC_LENGTH_REQUIRED_HDR;}
                case HttpResponse::SC_PRECONDITION_FAILED: { return SC_PRECONDITION_FAILED_HDR;}
                case HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE: { return SC_REQUEST_ENTITY_TOO_LARGE_HDR;}
                case HttpResponse::SC_REQUEST_URI_TOO_LONG: { return SC_REQUEST_URI_TOO_LONG_HDR;}
                case HttpResponse::SC_UNSUPPORTED_MEDIA_TYPE: { return SC_UNSUPPORTED_MEDIA_TYPE_HDR;}
                case HttpResponse::SC_REQUESTED_RANGE_NOT_SATISFIABLE: { return SC_REQUESTED_RANGE_NOT_SATISFIABLE_HDR;}
                case HttpResponse::SC_EXPECTATION_FAILED: { return SC_EXPECTATION_FAILED_HDR;}
                case HttpResponse::SC_INTERNAL_SERVER_ERROR: { return SC_INTERNAL_SERVER_ERROR_HDR;}
                case HttpResponse::SC_NOT_IMPLEMENTED: { return SC_NOT_IMPLEMENTED_HDR;}
                case HttpResponse::SC_BAD_GATEWAY: { return SC_BAD_GATEWAY_HDR;}
                case HttpResponse::SC_SERVICE_UNAVAILABLE: { return SC_SERVICE_UNAVAILABLE_HDR;}
                case HttpResponse::SC_GATEWAY_TIMEOUT: { return SC_GATEWAY_TIMEOUT_HDR;}
                case HttpResponse::SC_HTTP_VERSION_NOT_SUPPORTED: { return SC_HTTP_VERSION_NOT_SUPPORTED_HDR;}
            }
            return SC_INTERNAL_SERVER_ERROR_HDR;
        }

    } //namespace detail

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
    auto p = headers.find(hdrName);
    if(p != headers.end()) {
        return p->second.values;
    }
    return std::vector<std::string>();
}

void HttpMessage::clear() {
    contentLength = 0UL;
    contentType.clear();
    headers.clear();
    attributes.clear();
    cookies.clear();
}

namespace detail {

class CookieParser {
public:
    CookieParser(const std::string & str, HttpCookie *_pCookie ) : status(COOKIE_NAME), pCookie(_pCookie) {
        LOG(geryon::util::Log::DEBUG) << "Parsing cookie:" << str;
        char * pc = const_cast<char *>(str.c_str());
        while(*pc) {
            acceptChar(*pc);
            pc++;
        }
        acceptChar('\0'); //finish it up
    }

private:

    enum CookieState {
        COOKIE_NAME,
        COOKIE_VALUE,
        COOKIE_ATTR_NAME,
        COOKIE_PATH_VALUE,
        COOKIE_DOMAIN_VALUE,
        COOKIE_MAX_AGE_VALUE,
        COOKIE_EXPIRES_VALUE
    };

    void acceptChar(char c) {
        switch(status) {
            case COOKIE_NAME:
                if(c == '\0') {
                    return;
                } else if(c == '=') {
                    status = COOKIE_VALUE;
                } else {
                    pCookie->name.push_back(c);
                }
                break;
            case COOKIE_VALUE:
                if(c == '"') {
                    return;
                } else if(c == ';' || c == '\0') {
                    status = COOKIE_ATTR_NAME;
                } else {
                    pCookie->value.push_back(c);
                }
                break;
            case COOKIE_ATTR_NAME:
                if(c == ' ') {
                    return;
                }
                if(c == '=' || c == ';' || c == '\0') {
                    status = checkAttributeName();
                } else {
                    currentAttributeName.push_back(c);
                }
                break;
            case COOKIE_PATH_VALUE:
                if(c == '"') {
                    return;
                } else if(c == ';' || c == '\0') {
                    status = COOKIE_ATTR_NAME;
                } else {
                    pCookie->path.push_back(c);
                }
                break;
            case COOKIE_DOMAIN_VALUE:
                if(c == '"') {
                    return;
                } else if(c == ';' || c == '\0') {
                    status = COOKIE_ATTR_NAME;
                } else {
                    pCookie->domain.push_back(c);
                }
                break;
            case COOKIE_MAX_AGE_VALUE:
                if(c == '"') {
                    return;
                } else if(c == ';' || c == '\0') {
                    pCookie->maxAge = geryon::util::convertTo(currentValue, -1);
                    currentValue.clear();
                    status = COOKIE_ATTR_NAME;
                } else {
                    currentValue.push_back(c);
                }
                break;
            case COOKIE_EXPIRES_VALUE:
                if(c == '"') {
                    return;
                } else if(c == ';' || c == '\0') {
                    pCookie->expires = geryon::util::convertISODateTime(currentValue);
                    currentValue.clear();
                    status = COOKIE_ATTR_NAME;
                } else {
                    currentValue.push_back(c);
                }
                break;
        }
    }

    CookieState checkAttributeName() {
        CookieState nextState = COOKIE_ATTR_NAME;
        if(currentAttributeName == "Expires" || currentAttributeName == "expires") {
            nextState = COOKIE_EXPIRES_VALUE;
        } else if(currentAttributeName == "Domain" || currentAttributeName == "domain") {
            nextState = COOKIE_DOMAIN_VALUE;
        } else if(currentAttributeName == "Path" || currentAttributeName == "path") {
            nextState = COOKIE_PATH_VALUE;
        } else if(currentAttributeName == "Max-Age" || currentAttributeName == "max-age") {
            nextState = COOKIE_MAX_AGE_VALUE;
        } else if(currentAttributeName == "HttpOnly" || currentAttributeName == "httpOnly" || currentAttributeName == "httponly") {
            pCookie->httpOnly = true;
        } else if(currentAttributeName == "Secure" || currentAttributeName == "secure") {
            pCookie->secure = true;
        } else {
            pCookie->extension = currentAttributeName;
        }
        currentAttributeName.clear();
        return nextState;
    }

    CookieState status;
    HttpCookie * pCookie;
    std::string currentAttributeName;
    std::string currentValue;
};

}

const char * const COOKIE_HEADER_NAME = "Cookie";
const char * const SET_COOKIE_HEADER_NAME = "Set-Cookie";

bool HttpMessage::hasCookie(const std::string & cookieName) {
    if(cookies.find(cookieName) != cookies.end()) {
        return true;
    }
    std::string srch = cookieName + "=";

    //if not, explode the cookie
    auto cv = headers.find(COOKIE_HEADER_NAME);
    if(cv == headers.end()) {
        return false;
    }
    for(auto s : cv->second.values) {
        if(geryon::util::startsWith(s, srch)) {
            HttpCookie cookie;
            detail::CookieParser parser(s, &cookie);
            cookies.insert(std::make_pair(cookieName, std::move(cookie)));
            return true;
        }
    }
    return false;
}

bool HttpMessage::getCookie(const std::string & cookieName, HttpCookie & cookie) {
    if(!hasCookie(cookieName)) {
        return false;
    }
    auto p = cookies.find(cookieName);
    if(p != cookies.end()) {
        cookie = p->second;
        return true;
    }
    return false;
}

/* ====================================================================
 * HttpRequest implementation
 * ================================================================== */

void HttpRequest::clear() {
    HttpMessage::clear();

    method = "GET";
    methodCode = HttpMethodCode::GET;
    uri.clear();
    httpVersion.clear();
    uriProtocol.clear();
    queryString.clear();
    uriPath.clear();
    uriHost.clear();
    uriPort.clear();
    parameters.clear();

    parts.clear();
}

const std::string HttpRequest::getParameterValue(const std::string & paramName) const {
    auto p = parameters.find(paramName);
    if(p != parameters.end()) {
        if(p->second.size() > 0) {
            return p->second[0];
        }
    }
    return "";
}

const std::vector<std::string> HttpRequest::getParameterValues(const std::string & paramName) const {
    auto p = parameters.find(paramName);
    if(p != parameters.end()) {
        return p->second;
    }
    return std::vector<std::string>();
}

/* ====================================================================
 * HttpResponse implementation
 * ================================================================== */

void HttpResponse::addHeader(const std::string & hdrName, const std::string & hdrValue) throw (HttpException) {
    if(responseCommitted()) {
        std::string msg = "Header '" + hdrName + "' cannot be modified";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
    }
    std::map<std::string,HttpHeader>::iterator p = headers.find(hdrName);
    if(p == headers.end()) {
        HttpHeader hdr;
        hdr.name = hdrName;
        hdr.values.push_back(hdrValue);
        headers.insert(make_pair(hdrName, std::move(hdr)));
    } else {
        p->second.values.push_back(hdrValue);
    }
}

void HttpResponse::addCookie(const HttpCookie & cookie) throw (HttpException) {
    if(!cookie.isValid()) {
        std::string msg = "Cannot add malformed cookie '" + cookie.name + "'.";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::COOKIE_MALFORMED, msg);
    }
    std::ostringstream hv;
    hv << cookie.name << "=" << cookie.value;
    if(cookie.path.length()) {
        hv << "; Path=" << cookie.path;
    }
    if(cookie.domain.length()) {
        hv << "; Domain=" << cookie.domain;
    }
    if(cookie.maxAge >= 0) {
        hv << "; Max-Age=" << cookie.maxAge;
    }
    if(cookie.expires != 0) {
        std::string s = geryon::util::formatISODateTime(cookie.expires);
        hv << "; Expires=" << s;
    }
    if(cookie.secure) {
        hv << "; Secure";
    }
    if(cookie.httpOnly) {
        hv << "; HttpOnly";
    }
    if(cookie.extension.length()) {
        hv << ";" << cookie.extension;
    }
    addHeader(SET_COOKIE_HEADER_NAME, hv.str());
}

void HttpResponse::removeHeader(const std::string & hdrName) throw (HttpException) {
    if(responseCommitted()) {
        std::string msg = "Header '" + hdrName + "' cannot be deleted";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
    }
    headers.erase(hdrName);
}

void HttpResponse::setStatus(HttpStatusCode _status) throw (HttpException) {
    if(responseCommitted()) {
        std::string msg = "Status '" + std::to_string(_status) + "' cannot be set, already sent";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
    }
    status = _status;
}

void HttpResponse::clear() {
    HttpMessage::clear();
    status = HttpStatusCode::SC_OK;
    committed = false;
}

void HttpResponse::sendHeaders() throw (HttpException) {
    if(responseCommitted()) {
        std::string msg = "Response already committed";
        LOG(geryon::util::Log::ERROR) << msg;
        throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
    }
    committed = true;

    //1: status
    getOutputStream() << detail::getHeaderStatusMessage(status);
    //2: content length
    if(contentLength) {
        getOutputStream() << "Content-Length: " << std::to_string(contentLength) << "\r\n";
    }
    //3: content type
    if(contentType != "") {
        getOutputStream() << "Content-Type: " << contentType << "\r\n";
    } else {
        getOutputStream() << "Content-Type: text/plain\r\n";
    }
    //4: the rest of the headers
    for(std::map<std::string, HttpHeader>::iterator i = headers.begin(); i != headers.end(); ++i) {
        for(std::vector<std::string>::iterator j = i->second.values.begin(); j != i->second.values.end(); ++j) {
            getOutputStream() << i->second.name << ": " << *j << "\r\n";
        }
    }
    getOutputStream() << "\r\n";
}

}

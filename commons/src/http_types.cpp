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

}

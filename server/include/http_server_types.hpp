/**
 * \file http_server_types.cpp
 *
 *  Created on: May 16, 2014
 *      Author: rdumitriu at gmail dot com
 */

#ifndef HTTPSERVERTYPES_HPP_
#define HTTPSERVERTYPES_HPP_

#include "http_types.hpp"

namespace geryon { namespace server {

class HttpProtocolHandler;
class HttpRequestParser;

class HttpServerRequest : public HttpRequest {
public:
    ///
    /// Constructor of the request
    ///
    HttpServerRequest() : HttpRequest() {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpServerRequest() {}

    ///
    ///\brief The session
    ///\return a pointer to the session; if it does not exist, it is created.
    ///
    virtual Session * getSession() const { return 0; }

    ///
    ///\brief The input stream.
    ///
    ///\return the stream
    ///
    virtual std::istream & getInputStream() { return stream; }

    ///
    /// \brief Adds a header
    ///
    /// Adds a header; if the header already exists, it adds only the value.\n
    /// Do not add 'Content-Length' and 'Content-Type' headers ! These two are
    /// treated separately!
    ///
    /// \param hdrName the header name
    /// \param hdrValue the values
    ///
    void addHeader(const std::string & hdrName, const std::string & hdrValue);

    void addParameter(const std::string & prmName, const std::string & prmValue);

    void addPart(std::shared_ptr<HttpRequestPart> ptr);

    friend class HttpProtocolHandler;
    friend class HttpRequestParser;
private:
    std::istringstream stream;
};

class HttpServerRequestPart : public HttpRequestPart {
public:
    ///
    /// Constructor of the request
    ///
    HttpServerRequestPart(const std::string & _name, const std::string & _fileName, const std::string & _contentType)
                                : HttpRequestPart(_name, _fileName, _contentType) {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpServerRequestPart() {}
    ///
    ///\brief The input stream.
    ///
    ///\return the stream
    ///
    virtual std::istream & getInputStream() { return stream; }

    friend class HttpProtocolHandler;
    friend class HttpRequestParser;
private:
    std::istringstream stream;
};


class HttpServerResponse : public HttpResponse {
public:
    ///
    /// Constructor of the request
    ///
    HttpServerResponse() : HttpResponse() {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpServerResponse() {}

    ///
    /// \brief The output stream
    ///
    /// You will write here the response; if you send a lot of data (and I mean: a lot) you will need to call flush from
    /// time to time on the stream. Flushing the stream will send all the bytes on the wire and will lower the total
    /// memory consuption.
    ///
    virtual std::ostream & getOutputStream() { return stream; }

    friend class HttpProtocolHandler;
private:
    std::ostringstream stream;
};

} }

#endif

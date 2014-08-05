/**
 * \file http_server_types.hpp
 *
 *  Created on: May 16, 2014
 *      Author: rdumitriu at gmail dot com
 */

#ifndef GERYON_HTTPSERVERTYPES_HPP_
#define GERYON_HTTPSERVERTYPES_HPP_

#include <vector>

#include "http_types.hpp"
#include "mem_buf.hpp"
#include "mem_buf_input_stream.hpp"
#include "mem_buf_output_stream.hpp"

namespace geryon { namespace server {

class TCPProtocolHandler;
class HttpProtocolHandler;
class HttpRequestParser;
class HttpServerRequestPart;

class HttpServerRequest : public HttpRequest {
public:
    ///
    /// Constructor of the request
    ///
    HttpServerRequest() : HttpRequest(), buff(buffers), stream(&buff), startStreamIndex(0), endStreamIndex(0) {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpServerRequest() {}

    ///
    ///\brief The session
    ///\return a reference to the session; if it does not exist, it is created.
    ///
    virtual Session & getSession() const {
        if(session.get()) {
            return *session;
        }
        throw HttpException("Internal error: Session not set ?!?");
    }

    ///
    ///\brief The session setter
    ///
    void setSession(std::shared_ptr<Session> _session) {
        session = _session;
    }

    void setSessionCookie(const std::string & _sessionCookie) {
        sessionCookie = _sessionCookie;
    }

    void setConnection(std::shared_ptr<TCPConnection> _connection) {
        connection = connection;
    }

    const std::string & getSessionCookie() {
        return sessionCookie;
    }

    ///
    ///\brief The input stream.
    ///
    ///\return the stream
    ///
    virtual std::istream & getInputStream();

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

    ///
    /// \brief Adds a parameter
    ///
    /// \param prmName the parameter name
    /// \param prmValue the value
    ///
    void addParameter(const std::string & prmName, const std::string & prmValue);

    ///
    /// \brief Adds a part
    ///
    /// \param ptr the part shared pointer
    ///
    void addPart(std::shared_ptr<HttpRequestPart> ptr);

    void setStreamStartIndex(std::size_t n) { startStreamIndex = n; }

    friend class HttpProtocolHandler;
    friend class HttpRequestParser;
    friend class HttpServerRequestPart;
private:
    std::shared_ptr<TCPConnection> connection;
    std::vector<GBufferHandler> buffers;
    GIstreambuff buff;
    std::istream stream;
    std::size_t startStreamIndex;
    std::size_t endStreamIndex;
    std::string sessionCookie;
    std::shared_ptr<Session> session;
};

class HttpServerRequestPart : public HttpRequestPart {
public:
    ///
    /// Constructor of the request part
    ///
    HttpServerRequestPart(const std::string & _name, const std::string & _fileName, const std::string & _contentType,
                          HttpServerRequest * const _pServerRequest, std::size_t _startIndex, std::size_t _stopIndex)
                                : HttpRequestPart(_name, _fileName, _contentType),
                                  pServerRequest(_pServerRequest),
                                  buff(_pServerRequest->buffers, _startIndex, _stopIndex),
                                  stream(&buff) {}

    ///
    /// Destructor of the request part
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
    HttpServerRequest * pServerRequest;

    GIstreambuff buff;
    std::istream stream;
};


class HttpServerResponse : public HttpResponse {
public:
    ///
    /// Constructor of the request
    ///
    HttpServerResponse(TCPProtocolHandler * httpPHandler) : HttpResponse(), buff(httpPHandler), stream(&buff) {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpServerResponse() {}

    ///
    /// \brief The output stream
    ///
    /// You will write here the response.
    ///
    virtual std::ostream & getOutputStream() {
        if(!responseCommitted()) {
            sendHeaders();
        }
        return stream;
    }

    ///
    /// \brief flushes the stream.
    ///
    void flush() {
        if(!responseCommitted()) {
            sendHeaders();
        }
        stream.flush();
    }

    ///
    /// \brief close the stream (and flushes it)
    ///
    void close() {
        if(!responseCommitted()) {
            sendHeaders();
        }
        buff.close();
    }

    friend class HttpProtocolHandler;
private:
    ///Call this to format the preamble of any HTTP message. Sending the headers will make impossible to change
    /// afterwards any header, status, etc
    /// It changes also the committed flag
    void sendHeaders();

    GOstreambuff buff;
    std::ostream stream;
};

} }

#endif

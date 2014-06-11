///
/// \file http_types.hpp
///
///  Created on: Jul 7, 2011, refactored to geryon (and corrected bugs) on 24 Feb 2014
///      Author: rdumitriu at gmail.com
///

#ifndef HTTP_TYPES_HPP_
#define HTTP_TYPES_HPP_

#include <string>
#include <istream>
#include <ostream>
#include <map>
#include <vector>
#include <memory>
#include <chrono>
#include <stdexcept>

#include "log.hpp"
#include "string_utils.hpp"

#include <boost/any.hpp>


namespace geryon {

///
/// \brief A HTTP header may have more than one value.
///
///  HTTP headers may be defined twice in a request. There are some headers that
/// will not have any value but they carry one bit of information by mere presence.
///
struct HttpHeader {
    ///the name of the header
    std::string name;
    ///the value(s) of the header
    std::vector<std::string> values;
};

///
///  \brief The HTTP cookie, exploded.
///
///
/// The HTTP cookies support.\n\n
///
/// Partial support for RFC2109, skipped obsolete parts from RFC2965 and partial support for RFC6265.\n
///
/// Do not set both 'Expires' and 'Max-Age'. Doing so will result in a confusing cookie. We do not check for missing
/// name or value, so be sure you set it up correctly; if in doubt, call isValid() on the cookie.\n
///
/// It parses correctly only expires dates expressed as standard strings (i.e. yyyy-MMM-dd hh:mm:ss).\n\n
/// If this does not cover your needs, you should work directly with the header.
///
/// \todo Fully Support RFC6265 (even for cookies not set by this server)
///
struct HttpCookie {
    ///the name of the cookie; it must exist
    std::string name;
    ///the value of the cookie; it must exist
    std::string value;
    ///the declared path
    std::string path;
    ///the domain
    std::string domain;
    ///max age, in seconds.
    long maxAge;
    //expires, as time.
    std::time_t expires;
    ///set this to true if you don't want Javascript to play with the cookie
    bool httpOnly;
    ///Secure flag
    bool secure;
    ///extension
    std::string extension;
    
    ///Constructor: no version, no path, no max age, nothing
    HttpCookie() : maxAge(-1), expires(0L), httpOnly(false), secure(false) {}

    ///
    /// \brief Is this a valid cookie ?
    /// \return true if this is a valid cookie
    ///
    bool isValid() const {
        return (name.length() && value.length() &&
                    (maxAge < 0 || expires == 0L));
    }
};

///
/// \brief Exception if something goes wrong within the HTTP (incorrect usage, bad karma)
///
class HttpException : public std::runtime_error {
public:
    enum HttpExceptionCode {
        GENERIC_HTTP_ERROR,
        HEADERS_ALREADY_SENT,
        COOKIE_MALFORMED
    };

    ///
    /// \brief Constructor.
    ///
    /// \param msg the error message
    ///
    explicit HttpException(const std::string & msg) : std::runtime_error(msg) {}

    ///
    /// \brief Alternate constructor, with an error code
    ///
    /// \param code the code
    /// \param msg the additional message
    explicit HttpException(HttpExceptionCode code, const std::string & msg);

    ///
    /// \brief Alternate constructor, with an error code
    ///
    /// \param code the code
    explicit HttpException(HttpExceptionCode code);
    
    ///
    /// \brief Destructor
    ///
    virtual ~HttpException() throw() {}
};


///
/// \brief Base Http Message.
///
/// Messages should never be copied, same instance should be passed for processing.
/// Messages usually have buffers (I/O streams); they should not be in the base class, but in the subsequent
/// derivations. \n
/// Since HTTP message is a base class - and very abstract - it cannot be instantiated directly.\n\n
///
/// \todo Cookies improvements\n
///
class HttpMessage {
protected:
    /// \brief The HTTP message constructor
    HttpMessage() : contentLength(0) {}

     /// \brief The HTTP message destructor
    virtual ~HttpMessage() {}

public:
    /// Non-copyable
    HttpMessage(const HttpMessage & copy) = delete;

    /// Non-copyable
    HttpMessage & operator= (const HttpMessage & other) = delete;

    ///
    /// \brief Gets a certain header (first value)
    ///
    /// Returns the first value associated with a given header;
    /// \param hdrName the header name
    /// \return the value, if any, or an empty string if no header exists
    ///
    const std::string getHeaderValue(const std::string & hdrName) const;
    
    ///
    /// \brief Gets a certain header (all values)
    ///
    /// Returns the first value associated with a given header;
    /// \param hdrName the header name
    /// \return the values, if any, or an empty vector of strings if no header exists
    ///
    const std::vector<std::string> getHeaderValues(const std::string & hdrName) const;
    
    ///
    /// \brief Has a certain header ?
    ///
    /// Query to see if a header is already set on the message.
    ///
    /// \param hdrName the header name
    /// \return true if the header is there, false otherwise
    ///
    inline bool hasHeader(const std::string & hdrName) {
        auto p = headers.find(hdrName);
        return (p != headers.end());
    }
   
    ///
    /// \brief Gets the content length
    ///
    /// \return the content length
    ///
    inline std::size_t getContentLength() const { return contentLength; }
    
    ///
    /// \brief Gets the content type
    ///
    /// \return the content type
    ///
    inline const std::string & getContentType() const { return contentType; }
    
    ///
    /// \brief Basic support for cookies.
    ///
    /// Parses a header and returns the exploded cookie structure.\n\n
    ///
    /// Only the first cookie found is returned. For advanced cookie manipulation, please work directly with the header.
    ///
    ///
    /// \param cookieName the cookie name
    /// \return true if the cookie is parsed, false otherwise
    ///
    bool getCookie(const std::string & cookieName, HttpCookie &cookie);

    ///
    /// \brief Checks for cookie existence.
    ///
    /// Parses a header and returns the exploded cookie structure.\n\n
    ///
    /// Only the first cookie found is returned. For advanced cookie manipulation, please work directly with the header.
    ///
    ///
    /// \param cookieName the cookie name
    /// \return true if the cookie is parsed, false otherwise
    ///
    bool hasCookie(const std::string & cookieName);
    
    ///
    /// \brief Puts an attribute value.
    ///
    /// \param name the name of the value
    /// \param obj the value
    ///
    template <typename T>
    void putAttribute(const std::string & name, const T & obj) {
        auto p = attributes.find(name);
        if(p != attributes.end()) {
            attributes.erase(p);
        }
        attributes.insert(std::make_pair(name, obj));
    }

    ///
    /// \brief Gets the attribute value.
    ///
    /// \param name the name of the value
    /// \return the true if the get is made, otherwise false
    ///
    template <typename T>
    bool getAttribute(const std::string & name, T & obj) const {
        try {
            auto p = attributes.find(name);
            if(p != attributes.end()) {
                obj = boost::any_cast<T>(p->second);
                return true;
            }
        } catch(boost::bad_any_cast & e) {
            std::string msg = "Bad cast getting attribute:" + name;
            LOG(geryon::util::Log::ERROR) << msg;
        }
        return false;
    }
    
    ///
    /// \brief Removes a named attribute.
    ///
    /// \param name the name
    /// \return true if the attribute was removed, false otherwise
    ///
    inline bool removeAttribute(const std::string & name) {
        auto p = attributes.find(name);
        if(p != attributes.end()) {
            attributes.erase(p);
            return true;
        }
        return false;
    }

protected:
    /// \brief Reset the message for reuse
    virtual void clear();

    ///Headers
    std::map<std::string, HttpHeader> headers;
    ///Attributes
    std::map<std::string, boost::any> attributes;
    ///Content-Length header, int
    std::size_t contentLength;
    ///Content-Type header, the value
    std::string contentType;
private:
    ///Expanded cookies, per request basis
    std::map<std::string, HttpCookie> cookies;
};


///
///  \brief Support for multipart/form-data requests.
///
/// Multipart requests are standard form requests where enctype="multipart/form-data" They are usually used to post
/// files on servers.
///
class HttpRequestPart {
public:
    HttpRequestPart(const std::string & _name, const std::string & _fileName, const std::string & _contentType)
        : name(_name), fileName(_fileName), contentType(_contentType) {}

    ///Non-Copyable
    HttpRequestPart(const HttpRequestPart & copy) = delete;
    ///Non-Copyable
    HttpRequestPart & operator = (const HttpRequestPart & other) = delete;

    virtual ~HttpRequestPart() {}

    inline const std::string & getName() const { return name; }

    inline const std::string & getFileName() const { return fileName; }

    inline const std::string & getContentType() const { return contentType; }

    ///The real deal is here
    virtual std::istream & getInputStream() = 0;
protected:
    ///Content-Disposition name
    std::string name;
    ///Content-Disposition file name
    std::string fileName;
    ///Content-Type header, the value
    std::string contentType;
};

class Session;

///
/// \brief A HTTP request received from a client.
///
/// This is where your request parameters lies, along with potential useful headers.
///
class HttpRequest : public HttpMessage {
public:

    ///
    /// \brief The HTTP method
    ///
    enum HttpMethodCode {
        /// \brief GET HTTP method
        GET,
        /// \brief HEAD HTTP method
        HEAD,
        /// \brief POST HTTP method
        POST,
        /// \brief PUT HTTP method
        PUT,
        /// \brief DELETE HTTP method
        DELETE,
        /// \brief TRACE HTTP method
        TRACE,
        /// \brief OPTIONS HTTP method
        OPTIONS
    };

    ///
    /// Constructor of the request
    ///
    HttpRequest() : HttpMessage() {}

    ///
    /// Destructor of the request
    ///
    virtual ~HttpRequest() {}

    ///
    /// \brief Gets a parameter, either from query (if GET) or from the body of the message (if POST)
    ///
    /// This returns only the first value.
    ///
    /// \param paramName the parameter name
    /// \return the parameter value, or an empty string if it does not exist
    ///
    const std::string getParameterValue(const std::string & paramName) const;

    ///
    /// \brief Gets a parameter; if missing, it returns the default value
    ///
    /// Convenience method, saves typing; gets the parameter from GET/POST
    ///
    /// \param paramName the parameter name
    /// \param defaultValue, the value returned if the parameter is not found
    /// \return the value of the parameter
    ///
    template <typename T>
    const T getParameterValue(const std::string & paramName, const T & defaultValue) const {
        std::string s = getParameterValue(paramName);
        return geryon::util::convertTo(s, defaultValue);
    }

    ///
    /// \brief Gets a parameter, either from query (if GET) or from the body of the message (if POST)
    ///
    /// Returns all values found for that parameter
    ///
    /// \param paramName the parameter name
    /// \return the parameter values, or an empty string if it does not exist
    ///
    const std::vector<std::string> getParameterValues(const std::string & paramName) const;

    ///
    /// \brief Checks to see if a parameter is there
    /// \param name the name of the parameter.
    /// \return true if the parameter is there, false otherwise
    ///
    inline bool hasParameter(const std::string & name) const {
        auto p = parameters.find(name);
        return (p != parameters.end());
    }

    ///
    ///\brief The parsed URI protocol
    ///\return the parsed URI protocol (e.g. 'http'). It is not always filled!
    ///
    inline const std::string & getURIProtocol() const { return uriProtocol; }
    ///
    ///\brief The parsed URI host
    ///\return the parsed URI host (e.g. 'www.mydomain.com'). It is not always filled!
    ///
    inline const std::string & getURIHost() const { return uriHost; }
    ///
    ///\brief The parsed URI port
    ///\return the parsed URI port (e.g. '80'). It is not always filled!
    ///
    inline const std::string & getURIPort() const { return uriPort; }
    ///
    ///\brief The parsed URI path
    ///\return the parsed URI path. Always there.
    ///
    inline const std::string & getURIPath() const { return uriPath; }
    ///
    ///\brief The parsed URI QUERY
    ///\return the parsed URI query string. Always there.
    ///
    inline const std::string & getQueryString() const { return queryString; }
    ///
    ///\brief The parsed URI
    ///\return the URI.
    ///
    inline const std::string & getURI() const { return uri; }
    ///
    ///\brief The HTTP protocol version
    ///\return the version.
    ///
    inline const std::string & getHttpVersion() const { return httpVersion; }
    ///
    ///\brief The HTTP method, as a string
    ///\return the method string.
    ///
    inline const std::string & getMethod() const { return method; }
    ///
    ///\brief The HTTP method, as an int
    ///\return the method.
    ///
    inline HttpMethodCode getMethodCode() const { return methodCode; }

    ///
    ///\brief The parts of the message
    ///
    /// Because it's usually small, like headers, we do not mind copying some shared pointers.
    ///
    ///\return the parts vector
    ///
    inline const std::vector<std::shared_ptr<HttpRequestPart>> getParts() const { return parts; }

    ///
    ///\brief The session
    ///\return a reference to the session; if it does not exist, it is created.
    ///
    virtual Session & getSession() const = 0;

    ///
    ///\brief The input stream.
    ///
    ///\return the stream
    ///
    virtual std::istream & getInputStream() = 0;

protected:
    /// \brief Reset the message for reuse
    virtual void clear();

    std::string method; //filled
    std::string uri; //filled
    std::string httpVersion; //filled, HTTP/1.x
    std::string uriProtocol; //not always filled, http, https
    std::string queryString; //if present
    std::string uriPath; //uri path, always filled
    std::string uriHost; //not always filled
    std::string uriPort; //not always filled
    HttpMethodCode methodCode; //filled

    std::map<std::string, std::vector<std::string>> parameters;
    std::vector<std::shared_ptr<HttpRequestPart>> parts;
};


///
/// \brief A reply to be sent to a client.
///
/// This is what developers are allowed to modify. Operations are guarded, so you
/// cannot send something, then modify a header (a MessageException will pop up).
///
class HttpResponse : public HttpMessage {

public:
    ///
    /// \brief The status of the reply.
    ///
    /// HTTP error and sucess codes.
    ///
    enum HttpStatusCode {
        SC_CONTINUE = 100, //continue
        SC_SWITCHING_PROTOCOLS = 101, //server is acting as requested (upgrade header)
        SC_OK = 200, //ok
        SC_CREATED = 201, //resource created
        SC_ACCEPTED = 202, //accepted, but not completed
        SC_NON_AUTORITATIVE_INFO = 203, //content not originated from this server
        SC_NO_CONTENT = 204, //no info to return
        SC_RESET_CONTENT = 205, //browser should reset the document view
        SC_PARTIAL_CONTENT = 206, //partial content
        SC_MULTIPLE_CHOICES = 300,
        SC_MOVED_PERMANENTLY = 301, //moved, use the new location
        SC_MOVED_TEMPORARILY = 302, //moved, but use the original
        SC_SEE_OTHER = 303, //request can be found under different uri
        SC_NOT_MODIFIED = 304, //resource not modified
        SC_USE_PROXY = 305, //resource MUST be accessed through the proxy given by the Location field
        SC_BAD_REQUEST = 400, //bad request
        SC_UNAUTHORIZED = 401, //unauthorized
        SC_PAYMENT_REQUIRED = 402, //payment required
        SC_FORBIDDEN = 403, //forbidden
        SC_NOT_FOUND = 404, //not found
        SC_METHOD_NOT_ALLOWED = 405, //method specified in the Request-Line is not allowed for the resource identified by the Request-URI
        SC_NOT_ACCEPTABLE = 406, //not acceptable
        SC_PROXY_AUTHENTICATION_REQUIRED = 407, //request authentication proxy
        SC_REQUEST_TIMEOUT = 408, //request timeout
        SC_CONFLICT = 409, //conflict
        SC_GONE = 410, //gone
        SC_LENGTH_REQUIRED = 411, //please define Content-Length
        SC_PRECONDITION_FAILED = 412, //request-header fields evaluated to false when it was tested on the server
        SC_REQUEST_ENTITY_TOO_LARGE = 413, //big request, huh ?
        SC_REQUEST_URI_TOO_LONG = 414, //big uri, slash it.
        SC_UNSUPPORTED_MEDIA_TYPE = 415, //check content
        SC_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        SC_EXPECTATION_FAILED = 417, //server cannot comply with the Expect header
        SC_INTERNAL_SERVER_ERROR = 500, //server made a boo-boo
        SC_NOT_IMPLEMENTED = 501, //not implemented
        SC_BAD_GATEWAY = 502, //bad gateway
        SC_SERVICE_UNAVAILABLE = 503, //service unavailable
        SC_GATEWAY_TIMEOUT = 504, //gateway timeout
        SC_HTTP_VERSION_NOT_SUPPORTED = 505 //http version not supported
    };

    enum HttpContentType {
        CT_TEXT,
        CT_BINARY,
        CT_TEXTXML,
        CT_TEXTHTML,
        CT_APPLICATIONXML,
        CT_APPLICATIONJSON
    };

    ///
    /// Constructor of the reply
    ///
    HttpResponse() : HttpMessage(), status(HttpStatusCode::SC_OK), committed(false) {}

    ///
    /// Destructor of the reply
    ///
    virtual ~HttpResponse() {}

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
    void addHeader(const std::string & hdrName, const std::string & hdrValue) throw (HttpException);

    ///
    /// \brief Convenience method to add a 'Set-Cookie' header
    ///
    /// Creates a 'Set-Cookie' header and adds it on the response.
    /// \param cookie the cookie
    ///
    void addCookie(const HttpCookie & cookie) throw (HttpException);

    ///
    /// \brief Removes a certain header.
    ///
    /// If header exists, it is deleted (all values). If not, it does nothing
    ///
    /// \param hdrName the header name
    ///
    void removeHeader(const std::string & hdrName) throw (HttpException);

    ///
    /// \brief Sets the content length
    ///
    /// \param _contentLength the content length
    ///
    void setContentLength(std::size_t _contentLength) throw (HttpException) {
        if(responseCommitted()) {
            std::string msg = "Content-Length cannot be modified";
            LOG(geryon::util::Log::ERROR) << msg;
            throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
        }
        contentLength = _contentLength;
    }

    ///
    /// \brief Sets the content type
    ///
    /// \param contentType the content type
    ///
    void setContentType(const std::string & _contentType) throw (HttpException) {
        if(responseCommitted()) {
            std::string msg = "Content-Type cannot be modified";
            LOG(geryon::util::Log::ERROR) << msg;
            throw HttpException(HttpException::HEADERS_ALREADY_SENT, msg);
        }
        contentType = _contentType;
    }

    ///
    /// \brief Convenience method to set the content type
    /// \param ct the content type
    ///
    void setContentType(HttpContentType ct) throw (HttpException);

    ///
    /// \brief Sets the status
    ///
    /// \param status the status code (HTTPStatusCode::SC_X)
    ///
    void setStatus(HttpStatusCode _status) throw (HttpException);

    ///
    /// \brief Gets the status
    ///
    /// \return the status code (HTTPStatusCode::SC_X)
    ///
    inline HttpStatusCode getStatus() const {
        return status;
    }

    ///
    /// \brief The output stream
    ///
    /// You will write here the response; if you send a lot of data (and I mean: a lot) you will need to call flush from
    /// time to time on the stream. Flushing the stream will send all the bytes on the wire and will lower the total
    /// memory consuption.
    ///
    virtual std::ostream & getOutputStream() = 0;

protected:

    ///Clears the response for reuse
    virtual void clear();

    ///
    /// \brief Checks if the response is committed.
    ///
    /// Any attempt to modify the headers after the first byte sent should result in error
    /// \return true if the headers have been sent down the wire
    ///
    inline bool responseCommitted() {
        return committed;
    }

    HttpStatusCode status;
    bool committed;
};

const std::string & getHttpStatusMessage(HttpResponse::HttpStatusCode status);

}

#endif

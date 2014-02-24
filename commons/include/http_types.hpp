/*
 * http_types.hpp
 *
 *  Created on: Jul 7, 2011, refactored to geryon (and corrected bugs) on 24 Feb 2014
 *      Author: rdumitriu
 */

#ifndef HTTP_TYPES_HPP_
#define HTTP_TYPES_HPP_

#include <string>
#include <istream>
#include <map>
#include <vector>
#include <memory>
#include <chrono>
#include <stdexcept>

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
    std::chrono::time_point expires {0};
    ///set this to true if you don't want Javascript to play with the cookie
    bool httpOnly;
    ///Secure flag
    bool secure;
    ///extension
    std::string extension;
    
    ///Constructor: no version, no path, no max age, nothing
    HTTPCookie() : maxAge(-1), httpOnly(false), secure(false) {}
    /**
     * \brief Is this a valid cookie ?
     * \return true if this is a valid cookie
     */
    bool isValid() {
        return name.length() && value.length() &&
                    (maxAge < 0 || expires.time_since_epoch() == 0);
    }
};

/**
 * \brief Support for multipart/form-data requests.
 *
 * Multipart requests are standard form requests where enctype="multipart/form-data"
 * They are usually used to post files on servers.
 */
struct HttpRequestPart {
    ///Content-Disposition name
    std::string name;
    ///Content-Disposition file name
    std::string fileName;
    ///Content-Type header, the value
    std::string contentType;
    ///The real file name
    std::string realFileName;
};

///
/// \brief Exception if something goes wrong within the HTTP (incorrect usage, bad karma)
///
class HttpException : public std::runtime_error {
public:
    ///
    /// \brief Constructor.
    ///
    /// \param msg the error message
    ///
    explicit MessageException(const std::string & msg) : std::runtime_error(msg) {}

    ///
    /// \brief Alternate constructor, with an error code
    ///
    /// \param code the code
    /// \param msg the additional message
    explicit MessageException(HttpExceptionCode code, const std::string & msg);

    ///
    /// \brief Alternate constructor, with an error code
    ///
    /// \param code the code
    explicit MessageException(HttpExceptionCode code);
    
    /**
     * \brief Destructor
     */
    virtual ~MessageException() {}

    enum HttpExceptionCode {
        HEADERS_ALREADY_SENT
    };
};


///
/// \brief Base Http Message.
///
/// Messages should never be copied, same instance should be passed for processing.
/// Messages usually have buffers (I/O streams); they should not be in the base class, but in the implementations. \n
/// Since HTTP message is a base class - and very abstract - it cannot be instantiated directly.\n\n
///
/// \todo Cookies improvements\n
///
class HttpMessage {
protected:
    /// \brief The HTTP message constructor
    HttpMessage() {}

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
	const std::string & getHeaderValue(const std::string & hdrName) const;
    
    /**
     * \brief Gets a certain header (all values)
     * 
     * Returns the first value associated with a given header;
     * \param hdrName the header name
     * \return the values, if any, or an empty vector of strings if no header exists
     */
    const std::vector<std::string> & getHeaderValues(const std::string & hdrName) const;
    
    /**
     * \brief Has a certain header ?
     * 
     * Query to see if a header is already set on the message.
     * 
     * \param hdrName the header name
     * \return true if the header is there, false otherwise
     */
    bool hasHeader(const std::string & hdrName);
   
    /**
     * \brief Gets the content length
     * 
     * \return the content length
     */
    std::size_t getContentLength() const;
    
    /**
     * \brief Gets the content type
     * 
     * \return the content type
     */
    const std::string & getContentType() const;
    
	/**
     * \brief Basic support for cookies.
     * 
     * Parses a header and returns the exploded cookie structure.\n\n
     * 
     * Only the first cookie found is returned. For advanced cookie manipulation,
     * please work directly with the header.
     * 
     * \param cookieName the cookie name
     */
    HttpCookie getCookie(const std::string & cookieName);
    
    /**
     * \brief Puts an attribute value.
     *
     * \param name the name of the value
     * \param obj the value
     */
    template <typename T>
    std::shared_ptr<T> putAttribute(const std::string & name, T * obj) {
        std::map<std::string, ValueHolder *>::iterator p = m_attributes.find(name);
        if(p != m_attributes.end()) {
            TPValueHolder<T> *pSV = static_cast<TPValueHolder<T> *>(p->second);
            pSV->set(obj);
            return pSV->get();
        } else {
            TPValueHolder<T> *pData = new TPValueHolder<T>(obj);
            m_attributes.insert(std::make_pair(name, pData));
            return pData->get();
        }
    }

    /**
     * \brief Gets the attribute value.
     * 
     * \param name the name of the value
     * \return the value, wrapped in a shared pointer
     */
    template <typename T>
    std::shared_ptr<T> getAttribute(const std::string & name) {
        std::map<std::string, ValueHolder *>::iterator p = m_attributes.find(name);
        if(p != m_attributes.end()) {
            return (static_cast<TPValueHolder<T> *>(p->second))->get();
        }
        std::shared_ptr<T> ret(static_cast<T *>(NULL));
        return ret;
    }
    
    /**
     * \brief Removes a named attribute.
     * 
     * \param name the name
     */
    void removeAttribute(const std::string & name);
protected:
    ///Reset the message for reuse
    virtual void reset();

    ///Headers
    std::map<std::string, HTTPHeader> headers;
    ///Headers
    std::map<std::string, boost::any> attributes;
    ///Content-Length header, int
    std::size_t contentLength;
    ///Content-Type header, the value
    std::string contentType;
};





}

#endif

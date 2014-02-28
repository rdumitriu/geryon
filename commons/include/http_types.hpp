/**
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
    bool isValid() {
        return (name.length() && value.length() &&
                    (maxAge < 0 || expires == 0L));
    }
};

///
///  \brief Support for multipart/form-data requests.
///
/// Multipart requests are standard form requests where enctype="multipart/form-data" They are usually used to post
/// files on servers.
///
struct HttpRequestPart {
    ///Content-Disposition name
    std::string name;
    ///Content-Disposition file name
    std::string fileName;
    ///Content-Type header, the value
    std::string contentType;
    ///The real file name
    std::string realFileName; //::TODO:: this should be transformed into a stream !!! (and class!!)
};

///
/// \brief Exception if something goes wrong within the HTTP (incorrect usage, bad karma)
///
class HttpException : public std::runtime_error {
public:
    enum HttpExceptionCode {
        GENERIC_HTTP_ERROR,
        HEADERS_ALREADY_SENT
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
    
    /**
     * \brief Destructor
     */
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
    inline const std::string getContentType() const { return contentType; }
    
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
            removeAttribute(name);
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
            //::TODO:: log
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





}

#endif

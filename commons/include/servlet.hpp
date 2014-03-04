///
/// \file servlet.hpp
///
///  Created on: Dec 08, 2011, refactored on 1st of March 2014
///      Author: rdumitriu
///
#ifndef SERVLET_HPP_
#define SERVLET_HPP_

#include "http_types.hpp"
#include "appconfig_aware.hpp"

namespace geryon {

///
/// \brief The servlet.
///
/// The servlet is your principal way of interacting with the world at large.
/// One single instance of a servlet exists per application so your code MUST be thread safe.\n\n
///
/// For regex paths, application will prepend its own path, then the regex will be created. Regex expressions should be
/// marked providing the flag.
///
class Servlet : public ApplicationConfigAware {
protected:
    ///
    /// \brief Constructor.
    /// \param _path the path of the servlet. It is relative to the application.
    /// \param _regex true if the path is a REGEX
    ///
    explicit Servlet(const std::string & _path, bool _regex = false)
                                : ApplicationConfigAware(), path(_path), regex(_regex) {}
public:

    /// Non-copyable
    Servlet(const Servlet & copy) = delete;
    /// Non-copyable
    Servlet & operator = (const Servlet & other) = delete;

    ///
    /// \brief Destructor.
    ///
    virtual ~Servlet() {}

    ///
    /// \brief Gets the path back
    /// \return the path of the servlet
    ///
    inline std::string getPath() const { return path; }

    ///
    /// \brief Is the configured path a regex ?
    /// \return true if the the path is a REGEX
    ///
    inline bool isPathRegex() const { return regex; }

    ///
    /// \brief Called at initialization time.
    ///
    /// Here is where you usually initialize your servlet.
    ///
    virtual void init() {}

    ///
    /// \brief Executes the requests
    ///
    /// This is where the request is processed. The default behavior is to call one of the @code doXXX() @endcode
    /// methods depending on the method of the request.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void execute(HttpRequest * const pRequest, HttpResponse * const pReply);

    ///
    /// \brief Executes the requests - GET
    ///
    /// Implement this for GET. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doGet(HttpRequest * const pRequest, HttpResponse * const pReply) {}

    ///
    /// \brief Executes the requests - HEAD
    ///
    /// Implement this for HEAD. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doHead(HttpRequest * const pRequest, HttpResponse * const pReply) {}

    ///
    /// \brief Executes the requests - POST
    ///
    /// Implement this for POST. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doPost(HttpRequest * const pRequest, HttpResponse * const pReply) {}

    ///
    /// \brief Executes the requests - PUT
    ///
    /// Implement this for PUT. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doPut(HttpRequest * const pRequest, HttpResponse * const pReply) {}

    ///
    /// \brief Executes the requests - DELETE
    ///
    /// Implement this for DELETE. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doDelete(HttpRequest * const pRequest, HttpResponse * const pReply) {}


    ///
    /// \brief Executes the requests - TRACE
    ///
    /// Implement this for TRACE. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doTrace(HttpRequest * const pRequest, HttpResponse * const pReply) {}

    ///
    /// \brief Executes the requests - OPTIONS
    ///
    /// Implement this for OPTIONS. The default implementation does nothing.
    ///
    /// \param pRequest the HTTP request
    /// \param pReply the HTTP reply
    ///
    virtual void doOptions(HttpRequest * const pRequest, HttpResponse * const pReply) {}

private:
    ///The path, in the form '/the/path/name' or 'REGEX expression'
    std::string path;
    bool regex;
};

}  /*namespace*/
#endif

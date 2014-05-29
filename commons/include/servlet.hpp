///
/// \file servlet.hpp
///
///  Created on: Dec 08, 2011, refactored on 1st of March 2014
///      Author: rdumitriu at gmail.com
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
/// For matcher paths (i.e. paths containing '*'), application will prepend its own path, then the matcher will be
/// created.
///
class Servlet : public ApplicationConfigAware {
protected:
    ///
    /// \brief Constructor.
    /// \param _path the path of the servlet. It is relative to the application.
    ///
    explicit Servlet(const std::string & _path)
                                : ApplicationConfigAware(), path(_path) {}
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
    /// \brief Called at initialization time.
    ///
    /// Here is where you usually initialize your servlet.
    ///
    virtual void init() {}

    ///
    /// \brief Called just before stop.
    ///
    /// Here is where you usually clean it up.
    ///
    virtual void done() {}

    ///
    /// \brief Executes the requests
    ///
    /// This is where the request is processed. The default behavior is to call one of the @code doXXX() @endcode
    /// methods depending on the method of the request.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void execute(HttpRequest & request, HttpResponse & reply);

    ///
    /// \brief Executes the requests - GET
    ///
    /// Implement this for GET. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doGet(HttpRequest & request, HttpResponse & reply) {}

    ///
    /// \brief Executes the requests - HEAD
    ///
    /// Implement this for HEAD. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doHead(HttpRequest & request, HttpResponse & reply) {}

    ///
    /// \brief Executes the requests - POST
    ///
    /// Implement this for POST. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doPost(HttpRequest & request, HttpResponse & reply) {}

    ///
    /// \brief Executes the requests - PUT
    ///
    /// Implement this for PUT. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doPut(HttpRequest & request, HttpResponse & reply) {}

    ///
    /// \brief Executes the requests - DELETE
    ///
    /// Implement this for DELETE. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doDelete(HttpRequest & request, HttpResponse & reply) {}


    ///
    /// \brief Executes the requests - TRACE
    ///
    /// Implement this for TRACE. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doTrace(HttpRequest & request, HttpResponse & reply) {}

    ///
    /// \brief Executes the requests - OPTIONS
    ///
    /// Implement this for OPTIONS. The default implementation does nothing.
    ///
    /// \param request the HTTP request
    /// \param reply the HTTP reply
    ///
    virtual void doOptions(HttpRequest & request, HttpResponse & reply) {}

private:
    ///The path, in the form '/the/path/name' or '/the/path/*'
    std::string path;
};

}  /*namespace*/
#endif

/*
 * Servlet.hpp
 *
 *  Created on: Dec 08, 2011
 *      Author: rdumitriu
 */
#ifndef SERVLET_HPP_
#define SERVLET_HPP_

#include <string>

#include <HTTPTypes.hpp>

namespace agrade { namespace yahsrv {

//FWDs
class Application;
class ApplicationConfig;

/**
 * \brief The servlet.
 *
 * The servlet is your principal way of interacting with the world at large.
 * One single instance of a servlet exists per application so your code MUST be
 * thread safe.\n\n
 *
 * For regex paths, application will prepend its own path, then the regex will
 * be created. Regex expressions should be prefixed with @quot REGEX: @endquot.
 */
class Servlet {
private:
    /// Non-copyable
    Servlet(const Servlet & copy) = delete;
    /// Non-copyable
    Servlet & operator = (const Servlet & other) = delete;

    ///The path, in the form '/the/path/name' or 'REGEX:boost_regex_expression'
    std::string m_path;
public:
    /**
     * \brief Constructor.
     * \param path the path of the servlet. It is relative to the application.
     */
    Servlet(const std::string & path);

    /**
     * \brief Destructor.
     */
    virtual ~Servlet();

    /**
     * \brief Gets the path back
     * \return the path of the servlet
     */
    const std::string & getPath() const;

    /**
     * \brief Called at initialization time.
     *
     * Here is where you usually initialize your servlet.
     * \param app the application
     */
    virtual void init(Application & app);

    /**
     * \brief Executes the requests
     *
     * This is where the request is processed. The default behavior is to call
     * one of the @code doXXX() @endcode methods depending on the method of the
     * request.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void execute(HTTPRequest & request, HTTPReply & reply);

    /**
     * \brief Executes the requests - GET
     *
     * Implement this to serve the request on GET. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doGet(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - HEAD
     *
     * Implement this to serve the request on HEAD. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doHead(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - POST
     *
     * Implement this to serve the request on POST. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doPost(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - PUT
     *
     * Implement this to serve the request on PUT. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doPut(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - DELETE
     *
     * Implement this to serve the request on DELETE. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doDelete(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - TRACE
     *
     * Implement this to serve the request on TRACE. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doTrace(HTTPRequest & request, HTTPReply & reply);
    /**
     * \brief Executes the requests - OPTIONS
     *
     * Implement this to serve the request on OPTIONS. The default implementation
     * does nothing.
     *
     * \param request the HTTP request
     * \param reply the HTTP reply
     */
    virtual void doOptions(HTTPRequest & request, HTTPReply & reply);

    /**
     * \brief Gets the application config.
     *
     * Most of the time, you're interested in this application config.
     * \return the application config reference
     */
    ApplicationConfig & getAppConfig() const;

private:
    ApplicationConfig * m_pApplicationConfig;
};

}  } /*namespace*/
#endif

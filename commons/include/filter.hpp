/*
 * Filter.hpp
 *
 *  Created on: Dec 08, 2011
 *      Author: rdumitriu
 */
#ifndef FILTER_HPP_
#define FILTER_HPP_

#include <HTTPTypes.hpp>

namespace agrade { namespace yahsrv {

//FWDs
class Application;
class ApplicationConfig;

/**
 * \brief The filter.
 *
 * A filter filters the requests. The developer has the possibility of
 * interrupting the processing.
 * The default implementation does really nothing, since we do not know what
 * kind of processing would you like to add. So we do not add any regex here,
 * you can subclass it afterwards (or check the provided subclasses).
 */
class Filter {
public:
    /**
     * \brief The constructor
     */
    Filter();
    /**
     * \brief The destructor
     */
    virtual ~Filter();

    /**
     * \brief Called at initialization time.
     *
     * If you need some properties, or some SQL connection, here's the time to
     * initialize the filter.
     * \param app the application
     */
    virtual void init(Application & app);

    /**
     * \brief Actual filtering routine.
     *
     * Filters the requests. Returns true if the processing must continue, false otherwise
     * \param request the request
     * \param reply the reply
     * \return true or false, true to continue processing.
     */
    virtual bool doFilter(HTTPRequest & request, HTTPReply & reply);

    /**
     * \brief Gets the application config.
     *
     * Most of the time, you're interested in this application config.
     * \return the application config reference
     */
    ApplicationConfig & getAppConfig() const;

private:
    ApplicationConfig * m_pApplicationConfig;

    /// Non-copyable
    Filter(const Filter & copy) = delete;

    /// Non-copyable
    Filter & operator= (const Filter & other) = delete;
};

} } /* namespace */

#endif

///
/// \file filter.hpp
///
///  Created on: Dec 08, 2011, refactored on 2nd of March 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_FILTER_HPP_
#define GERYON_FILTER_HPP_

#include "http_types.hpp"
#include "appconfig_aware.hpp"

namespace geryon {

class ApplicationModule;
///
/// \brief The filter.
///
/// A filter filters the requests. The developer has the possibility of interrupting the processing.
/// The default implementation does really nothing, since we do not know what kind of processing you would like to
/// add.\n\n
///
/// Paths may contain '*' symbols to signal that accepts everything on that position.
///
class Filter : public ApplicationConfigAware {
public:
    ///
    /// \brief The constructor
    ///
    /// \param _path the path of the filter. It is relative to the application.
    ///
    Filter(const std::string & _path)
                        : ApplicationConfigAware(), path(_path)  {}
    ///
    /// \brief The destructor
    ///
    virtual ~Filter() {}

    /// Non-copyable
    Filter(const Filter & copy) = delete;

    /// Non-copyable
    Filter & operator= (const Filter & other) = delete;

    ///
    /// \brief Gets the path back
    /// \return the path of the filter, if any
    ///
    inline std::string getPath() const { return path; }

    ///
    /// \brief Called at initialization time.
    ///
    /// If you need some properties, or some SQL connection, here's the time to
    /// initialize the filter.
    /// \param app the application
    ///
    virtual void init() {}

    ///
    /// \brief Called just before stop.
    ///
    /// Here is where you usually clean it up.
    ///
    virtual void done() {}

    ///
    /// \brief Actual filtering routine.
    ///
    /// Filters the requests. Returns true if the processing must continue, false otherwise
    /// \param request the request
    /// \param reply the reply
    /// \return true or false, true to continue processing.
    ///
    virtual bool doFilter(HttpRequest & request, HttpResponse & reply) = 0;
private:
    std::string path;
};

} /* namespace */

#endif

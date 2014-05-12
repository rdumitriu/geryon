///
/// \file session_listener.hpp
///
///  Created on: March 4, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef SESSION_LISTENER_HPP_
#define SESSION_LISTENER_HPP_
 
#include <string>
#include <boost/any.hpp>

#include "appconfig_aware.hpp"

namespace geryon {

//FWDs
class Session;

///
/// \brief Signals the start/end/modify of a session
///
/// Such a listener should be configured before the application starts. \n
/// Code MUST be thread safe. The proper access is required (get the mutex)\n\n
/// We prefer to keep listeners as classes with behavior instead of having pointers to functions for that matter.
///
class SessionLifecycleListener : public ApplicationConfigAware {
public:
    ///
    /// \brief Constructor
    ///
    SessionLifecycleListener() : ApplicationConfigAware() {}
    ///
    /// \brief Destructor
    ///
    virtual ~SessionLifecycleListener() {}

    ///Non-copyable
    SessionLifecycleListener(const SessionLifecycleListener & copy) = delete;
    ///Non-copyable
    SessionLifecycleListener & operator= (const SessionLifecycleListener & other) = delete;

    ///
    /// \brief Called at initialization time
    ///
    /// \param pSes the session
    ///
    virtual void init(Session & ses) = 0;

    ///
    /// \brief Called when a value is added on a session
    ///
    /// You will need to cast the data to your real data
    ///
    /// \param pSes the session
    /// \param name the name of the variable
    /// \param val the data
    ///
    virtual void added(Session & ses,
                       const std::string & name,
                       const boost::any & val) = 0;
    ///
    /// \brief Called when a value is altered on a session
    ///
    /// You will need to cast the data to your real data.
    ///
    /// \param pSes the session
    /// \param name the name of the variable
    /// \param oldVal the data, old
    /// \param newVal the data, new
    ///
    virtual void modified(Session & ses,
                          const std::string & name,
                          const boost::any & oldVal, const boost::any & newVal) = 0;
    ///
    /// \brief Called when a values is removed on a session
    ///
    /// \param pSes the session
    /// \param name the name of the variable
    ///
    virtual void removed(Session & ses, const std::string & name) = 0;

    ///
    /// \brief Called when we called invalidate
    ///
    /// \param pSes the session
    ///
    virtual void invalidated(Session & ses) = 0;

    ///
    /// \brief Called when we're done with the session
    ///
    /// \param pSes the session
    ///
    virtual void done(Session & ses) = 0;
};

///
/// \brief Convenience class to write a session listener
///
/// All methods are empty. You need to subclass it before use \n
///
class SessionLifecycleListenerAdapter : public SessionLifecycleListener {
protected:
    ///Protected constructor, you need to subclass to use
    SessionLifecycleListenerAdapter() : SessionLifecycleListener() {}
public:
    ///Destructor
    virtual ~SessionLifecycleListenerAdapter() {}

    ///Initialization. This implementation does nothing.
    virtual void init(Session & ses) {}
    ///Attribute added. This implementation does nothing.
    virtual void added(Session & ses, const std::string & name, const boost::any & val) {}
    ///Attribute modified. This implementation does nothing.
    virtual void modified(Session & ses, const std::string & name,
                          const boost::any & oldVal, const boost::any & newVal) {}
    ///Attribute removed. This implementation does nothing.
    virtual void removed(Session & ses, const std::string & name) {}
    ///Session invalidated. This implementation does nothing.
    virtual void invalidated(Session & ses) {}
    ///Session will be soon gone. This implementation does nothing.
    virtual void done(Session & ses) {}
};

}

#endif

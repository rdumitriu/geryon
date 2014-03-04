///
/// \file application.hpp
///
///  Created on: Dec 08, 2011
///      Author: rdumitriu
///
#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_
 
#include <string>
#include <map>
#include <vector>

#include "filter.hpp"
#include "servlet.hpp"
#include "session_listener.hpp"
#include "session.hpp"

namespace geryon {

// ::TODO:: contains the pools, the config, etc
//
class ApplicationContext {

};

class ApplicationModule;
///
/// \brief Signals the start / end of the application module.
///
/// The application module must be configured when started with all the listeners.\n
/// Code MUST be thread safe
///
class ApplicationModuleLifecycleListener : public ApplicationConfigAware {
public:
    ///
    /// Constructor
    ///
    ApplicationModuleLifecycleListener() : ApplicationConfigAware() {}

    ///
    /// The destructor
    ///
    virtual ~ApplicationModuleLifecycleListener() {}

    ///Non-copyable
    ApplicationModuleLifecycleListener(const ApplicationModuleLifecycleListener & copy) = delete;
    ///Non-copyable
    ApplicationModuleLifecycleListener & operator= (const ApplicationModuleLifecycleListener & other) = delete;

    ///
    /// Called immediately after init.
    /// This is the place where you may initialize application-wide resources
    ///
    virtual void init() = 0;

    ///
    /// Called just before shutdown
    /// Usually you destroy here the application-wide resources you initialized
    ///
    virtual void done() = 0;
};

///
/// \brief Any application is a module, a container keeping servlets, filters, etc.
///
/// The application and the plugins are all application modules. Any application module has a config and a
/// certain path (to keep things separated). A key is provided to resolve modules by name.\n\n
///
/// Take care: when destroyed, this objects destroys all servlets, filters, and listeners as well. So don't worry
/// about deletes.
/// ::TODO:: add configuration in HERE !!!
///
class ApplicationModule {
protected:
    ///
    /// \brief The basic application block.
    ///
    /// We want an application be composed from multiple modules, but these share the same behavior
    ///
    /// \param _key the key of the module
    /// \param _parent the parent
    ///
    ApplicationModule(const std::string & _key, ApplicationModule * const _parent = NULL)
                        : key(_key), parent(_parent), status(ApplicationModule::INIT) {}
public:

    ///Destructor
    virtual ~ApplicationModule();

    enum Status {
        /// Configuration phase: all modules are born this way
        INIT,
        /// Started phase: Module is alive
        STARTED,
        /// Stopped.
        STOPPED
    };

    ///Non-Copyable
    ApplicationModule(const ApplicationModule & copy) = delete;
    ///Non-Copyable
    ApplicationModule & operator = (const ApplicationModule & other) = delete;

    ///Gets the key of the module
    inline std::string getKey() const { return key; }

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void start();

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void stop();

    ///
    /// \brief This object's lifecycle listener.
    ///
    /// Adds a listener on the application lifecycle.
    ///
    /// Listeners on the lifecycle of the application are owned by this object.\n
    /// Once added, the listener will be destroyed by the application destructor
    ///
    /// \param listener the listener address (MUST be pointer, MUST be allocated with 'new')
    ///
    void addLifecycleListener(ApplicationModuleLifecycleListener * const listener) {
        listener->setApplicationModule(this);
        moduleListeners.push_back(listener);
    }

    ///
    /// \brief Session lifecycle listener.
    ///
    /// Adds a listener on the session lifecycle.
    ///
    /// Listeners on the lifecycle of the session are owned by this object.\n
    /// Once added, the listener will be destroyed by the application destructor
    ///
    /// \param listener the listener address (MUST be pointer, MUST be allocated with 'new')
    ///
    void addSessionListener(SessionLifecycleListener * const listener) {
        listener->setApplicationModule(this);
        sessionListeners.push_back(listener);
    }


    ///
    /// \brief Filters.
    ///
    /// Adds a filter into the application.\n
    ///
    /// Filters intercept the request and they are able to interrupt the processing of a request.\n
    ///
    /// Filters are added on the application module and they are owned by the application module object.
    /// \param filter the filter pointer (allocated with 'new')
    ///
    void addFilter(Filter * const filter) {
        filter->setApplicationModule(this);
        filters.push_back(filter);
    }

    ///
    /// \brief Servlets.
    ///
    /// Adds a servlet into the application.\n
    ///
    /// Servlets are the main processing units. \n
    /// Servlets are added on the application and they are owned by the application module object, and subsequently
    /// destroyed by the current application module object.\n
    /// \param servlet the servlet pointer (allocated with 'new')
    ///
    void addServlet(Servlet * const servlet) {
        servlet->setApplicationModule(this);
        servlets.push_back(servlet);
    }

    ///Gets the status. Not thread-safe!
    inline Status getStatus() {
        return status;
    }
protected:

    ///Get the parent of this module, if any
    inline ApplicationModule * getParent() const { return parent; }

    friend class Session;

    ///Perf: To avoid copying values, use this just before notifying
    virtual bool mustNotifyForSessions();
    ///Notify: session created
    virtual void notifySessionCreated(Session * const pSes);
    ///Notify: session invalidated
    virtual void notifySessionInvalidated(Session * const pSes);
    ///Notify: session destroyed
    virtual void notifySessionDestroyed(Session * const pSes);
    ///Notify: session value added
    virtual void notifySessionValueAdded(Session * const pSes, const std::string & name, const boost::any & newVal);
    ///Notify: session value modified
    virtual void notifySessionValueChanged(Session * const pSes, const std::string & name, const boost::any & oldVal, const boost::any & newVal);
    ///Notify: session value removed
    virtual void notifySessionValueRemoved(Session * const pSes, const std::string & name);

    std::string key;
    ApplicationModule * parent;
    Status status;
    std::vector<Filter *> filters;
    std::vector<Servlet *> servlets;
    std::vector<ApplicationModuleLifecycleListener *> moduleListeners;
    std::vector<SessionLifecycleListener *> sessionListeners;
};

///
/// \brief A module of modules
///
class ApplicationModuleContainer : public ApplicationModule {
public:
    ApplicationModuleContainer(const std::string & _key, ApplicationModule * const _parent = NULL)
                                                                            : ApplicationModule(_key, _parent) {}

    virtual ~ApplicationModuleContainer() {}

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void start();

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void stop();

protected:
    ///Perf: To avoid copying values, use this just before notifying
    virtual bool mustNotifyForSessions();
    ///Notify: session created
    virtual void notifySessionCreated(Session * const pSes);
    ///Notify: session invalidated
    virtual void notifySessionInvalidated(Session * const pSes);
    ///Notify: session destroyed
    virtual void notifySessionDestroyed(Session * const pSes);
    ///Notify: session value added
    virtual void notifySessionValueAdded(Session * const pSes, const std::string & name, const boost::any & newVal);
    ///Notify: session value modified
    virtual void notifySessionValueChanged(Session * const pSes, const std::string & name, const boost::any & oldVal, const boost::any & newVal);
    ///Notify: session value removed
    virtual void notifySessionValueRemoved(Session * const pSes, const std::string & name);

};

typedef ApplicationModuleContainer Application;
typedef ApplicationModuleContainer Plugin;
    
///**
// * \brief The application part exposed to filters, servlets and listeners.
// *
// * Part of the application behavior is defined here. You should note that this
// * class does not provide any execution method (to avoid calling execute
// * inadverdently).
// *
// * The class defines the basics for an application to take shape: listeners,
// * the pools (via the configuration), servlets and filters.
// *
// * All servlets and filters will be relative to the base path of the application.
// * If the user fills in the configuration property 'mount.path' it will
// * have precedence over the configured mount requested by the programmer.
// */
//class Application {
//public:
//    /**
//     * \brief Constructor.
//     *
//     * \param path the base path of the application.
//     * \param configFile the config file of the application
//     */
//    Application(const std::string & path, const std::string & configFile);
    
//    /**
//     * \brief Destructor.
//     *
//     * Responsible for re-collecting all the memory occupied by
//     * servlets, filters, listeners.
//     */
//    virtual ~Application();
    
//    /**
//     * \brief Mounted path of the application (base path).
//     *
//     * Gets the path mapped on this application.
//     *
//     * Each path must start with a '/' and end with one i.e. '/myapp/'. If not
//     * automatic correction will take place.
//     * \return the path
//     */
//    const std::string & getPath() const;
    
//    /**
//     * \brief The configuration.
//     *
//     * Gets the configuration for this application.
//     *
//     * \return the configuration reference.
//     */
//    ApplicationConfig & getConfig();

//    /**
//     * \brief This object's lifecycle listener.
//     *
//     * Adds a listener on the application lifecycle.
//     *
//     * Listeners on the lifecycle of the application are owned by this object.
//     * Once added, the listener will be destroyed by the application destructor
//     *
//     * \param listener the listener address (MUST be pointer, MUST be allocated
//     * with 'new')
//     */
//    void addLifecycleListener(ApplicationLifecycleListener * listener);
    
//    /**
//     * \brief Session lifecycle listener.
//     *
//     * Adds a listener on the session lifecycle.
//     *
//     * Listeners on the lifecycle of the session are owned by this object.
//     * Once added, the listener will be destroyed by the application destructor
//     *
//     * \param listener the listener address (MUST be pointer, MUST be allocated
//     * with 'new')
//     */
//    void addSessionListener(SessionLifecycleListener * listener);
    
//    /**
//     * \brief Filters.
//     *
//     * Adds a filter into the application.
//     *
//     * Filters intercept the request and they are able to interrupt the
//     * processing of a request.
//     * Filters are added on the application and they are owned by the
//     * application object.
//     * \param filter the filter pointer (allocated with 'new')
//     */
//    void addFilter(Filter * filter);
    
//    /**
//     * \brief Servlets.
//     *
//     * Adds a servlet into the application.
//     *
//     * Servlets are the main processing units.
//     * Servlets are added on the application and they are owned by the
//     * application object, and subsequently destroyed by the current application
//     * object.\n\n
//     * Servlet path may contain a regex expression; it will be concatenated to the
//     * base path and then created a regex; the regex must start with "REGEX:"
//     * for us to be able to detect this is a regex expression.
//     * \param servlet the servlet pointer (allocated with 'new')
//     */
//    void addServlet(Servlet * servlet);

//protected:
//    ///Notify: session created
//    void notifySessionCreated(Session & ses);
//    ///Notify: session value added
//    void notifySessionValueAdded(Session & ses, const std::string & name, ValueHolder *pData);
//    ///Notify: session value modified
//    void notifySessionValueChanged(Session & ses, const std::string & name, ValueHolder *pData);
//    ///Notify: session value removed
//    void notifySessionValueRemoved(Session & ses, const std::string & name);
//    ///Notify: session value destroyed
//    void notifySessionDestroyed(Session & ses);
//    ///Notify: application created
//    void notifyApplicationStarted();
//    ///Notify: application killed
//    void notifyApplicationStopped();
//    ///Gets the servlet mapped on the path
//    Servlet * getServlet(const std::string & path) const;
    
//    friend class Session;

//    ///Application lifecycle listeners
//    std::vector<ApplicationLifecycleListener *> m_appLifecycleListeners;
//    ///Session lifecycle listeners
//    std::vector<SessionLifecycleListener *> m_sessLifecycleListeners;
//    ///Filters
//    std::vector<Filter *> m_filters;
//    ///Servlets : part 1: direct mapped servlets
//    std::map<std::string, Servlet *> m_servlets;
//    ///Servlets : part 2: regex mapped servlets (slower)
//    std::vector<agrade::yahsrv::internal::RegexServletHolder *> m_regexServlets;
    
//    ///The path, in the form '/path_name/'
//    std::string m_path;
//    ///Config: the config of the application
//    ApplicationConfig m_config;
//private:
//    /// Non-copyable
//    Application(const Application & copy) = delete;

//    /// Non-copyable
//    Application & operator= (const Application & other) = delete;
//};

//}


///**
// * \brief The plugin adds behavior.
// *
// * Programmers will add listeners, servlets, filters then configure the
// * application to copy everything inside it.\n
// * All servlets and filters will be relative to the base path of the
// * application, as usual.
// */
//class Plugin {
//public:
//    /**
//     * \brief Constructor.
//     */
//    Plugin();
    
//    /**
//     * \brief Destructor.
//     *
//     * Responsible for re-collecting all the memory occupied by
//     * servlets, filters, listeners, if after the wiring remains any.
//     */
//    virtual ~Plugin();

//    /**
//     * \brief Application lifecycle listener.
//     *
//     * Adds a listener on the application lifecycle.
//     * \param listener the listener address (MUST be pointer, MUST be allocated
//     * with 'new')
//     */
//    void addLifecycleListener(ApplicationLifecycleListener * listener);
    
//    /**
//     * \brief Session lifecycle listener.
//     *
//     * Adds a listener on the session lifecycle.
//     *
//     * \param listener the listener address (MUST be pointer, MUST be allocated
//     * with 'new')
//     */
//    void addSessionListener(SessionLifecycleListener * listener);
    
//    /**
//     * \brief Filters.
//     *
//     * Adds a filter.
//     *
//     * \param filter the filter pointer (allocated with 'new')
//     */
//    void addFilter(Filter * filter);
    
//    /**
//     * \brief Servlets.
//     *
//     * Adds a servlet into the plugin.
//     *
//     * Servlets are the main processing units.
//     *
//     * \param servlet the servlet pointer (allocated with 'new')
//     */
//    void addServlet(Servlet * servlet);

//private:
//    friend class ServerApplication;

//    /// Non-copyable
//    Plugin(const Plugin & copy) = delete;
//    /// Non-copyable
//    Plugin & operator = (const Plugin & other) = delete;
    
//    ///Application lifecycle listeners
//    std::vector<ApplicationLifecycleListener *> m_appLifecycleListeners;
//    ///Session lifecycle listeners
//    std::vector<SessionLifecycleListener *> m_sessLifecycleListeners;
//    ///Filters
//    std::vector<Filter *> m_filters;
//    ///Servlets
//    std::vector<Servlet *> m_servlets;
//};

}

#endif

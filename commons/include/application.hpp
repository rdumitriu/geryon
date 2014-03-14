///
/// \file application.hpp
///
///  Created on: Dec 08, 2011, remodified
///      Author: rdumitriu at gmail.com
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

#include "application_config.hpp"

namespace geryon {

//FWDs
class ApplicationModule;
class ApplicationModuleContainer;

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
///
class ApplicationModule {
public:
    ///
    /// \brief The basic application block.
    ///
    /// We want an application be composed from multiple modules, but these share the same behavior
    ///
    /// \param _key the key of the module
    /// \param _configFile the configuration file
    /// \param _parent the parent
    ///
    ApplicationModule(const std::string & _key,
                      const std::string & _configFile = "",
                      ApplicationModule * const _parent = 0);

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

    ///Gets the configuration of this module
    inline const ApplicationConfig & getConfig() { return config; }

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
    friend class ApplicationModuleContainer; //that's funny

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
    ApplicationConfig config;
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
    ApplicationModuleContainer(const std::string & _key,
                               const std::string & _configFile = "",
                               ApplicationModule * const _parent = 0)
                                        : ApplicationModule(_key, _configFile, _parent) {}

    virtual ~ApplicationModuleContainer();

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void start();

    ///Called by the container. By default, notifies all the configured listeners of the module.
    virtual void stop();

    //::TODO:: start / stop modules individually. At some point in time, soon

    ///
    /// \brief Adds a module
    /// \param module the module pointer
    ///
    void addModule(ApplicationModule * const module) {
        module->parent = this;
        modules.push_back(module);
    }

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
    virtual void notifySessionValueChanged(Session * const pSes, const std::string & name,
                                           const boost::any & oldVal, const boost::any & newVal);
    ///Notify: session value removed
    virtual void notifySessionValueRemoved(Session * const pSes, const std::string & name);

    std::vector<ApplicationModule *> modules;
};

///
/// \brief Application
///
/// By convention, an application is the root of all
///
typedef ApplicationModuleContainer Application;
///
/// \brief Plugin
///
/// By convention, a plugin is a simple module
///
typedef ApplicationModule Plugin;
///
/// \brief PluginContainer
///
/// By convention, the plugin container can accomodate other plugins.
///
typedef ApplicationModuleContainer PluginContainer;

}

#endif

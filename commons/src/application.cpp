/**
 * \file http_types.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: rdumitriu at gmail dot com
 */
#include "filter.hpp"
#include "servlet.hpp"
#include "session.hpp"
#include "application.hpp"

#include "string_utils.hpp"
#include "log.hpp"

//::TODO:: BIG Q: shall we run in isolation (try-catch) all the listeners and stuff ?

namespace geryon {

/* ====================================================================
 * ApplicationModule
 * ================================================================== */

ApplicationModule::ApplicationModule(const std::string & _key, ApplicationModule * const _parent)
                    : key(_key), config(), parent(_parent), status(ApplicationModule::INIT) {
}

ApplicationModule::~ApplicationModule() {
}

void ApplicationModule::start() {
    if(status == ApplicationModule::STARTED) {
        return;
    }
    for(auto p : moduleListeners) {
        p->init();
    }
    for(const auto p : filters) {
        p->init();
    }
    for(auto p : servlets) {
        p->init();
    }
    status = ApplicationModule::STARTED;
    LOG(geryon::util::Log::INFO) << "Started module " << key;
}

void ApplicationModule::stop() {
    if(status != ApplicationModule::STARTED) {
        return;
    }
    for(auto p : servlets) {
        p->done();
    }
    for(auto p : filters) {
        p->done();
    }
    for(auto p : moduleListeners) {
        p->done();
    }
    status = ApplicationModule::STOPPED;
    LOG(geryon::util::Log::INFO) << "Stopped module " << key;
}

bool ApplicationModule::mustNotifyForSessions() {
    return (!sessionListeners.empty() && status == ApplicationModule::STARTED);
}

void ApplicationModule::notifySessionCreated(Session & ses) {
    for(auto p : sessionListeners) {
        p->init(ses);
    }
}

void ApplicationModule::notifySessionInvalidated(Session & ses) {
    for(auto p : sessionListeners) {
        p->invalidated(ses);
    }
}

void ApplicationModule::notifySessionDestroyed(Session & ses) {
    for(auto p : sessionListeners) {
        p->done(ses);
    }
}

void ApplicationModule::notifySessionValueAdded(Session & ses, const std::string & name,
                                                const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->added(ses, name, newVal);
    }
}

void ApplicationModule::notifySessionValueChanged(Session & ses, const std::string & name,
                                                  const boost::any & oldVal, const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->modified(ses, name, oldVal, newVal);
    }
}

void ApplicationModule::notifySessionValueRemoved(Session & ses, const std::string & name) {
    for(auto p : sessionListeners) {
        p->removed(ses, name);
    }
}

void ApplicationModule::getFilters(std::vector<std::shared_ptr<Filter>> & v) const {
    LOG(geryon::util::Log::DEBUG) << "Returning " << filters.size() << " filters.";
    v.insert(v.end(), filters.begin(), filters.end());
}

void ApplicationModule::getServlets(std::vector<std::shared_ptr<Servlet>> & v) const {
    LOG(geryon::util::Log::DEBUG) << "Returning " << servlets.size() << " servlets.";
    v.insert(v.end(), servlets.begin(), servlets.end());
}

/* ====================================================================
 * ApplicationModuleContainer
 * ================================================================== */

ApplicationModuleContainer::ApplicationModuleContainer(const std::string & _key,
                                                       ApplicationModule * const _parent)
                                 : ApplicationModule(_key, _parent) {}

ApplicationModuleContainer::~ApplicationModuleContainer() {
}

void ApplicationModuleContainer::start() {
    ApplicationModule::start();
    for(auto mp : modules) {
        try {
            mp->start();
        } catch(std::runtime_error & e) {
            LOG(geryon::util::Log::ERROR) << "Module " << mp->key << " failed to initialize properly :" << e.what();
        } catch( ... ) {
            LOG(geryon::util::Log::ERROR) << "Module " << mp->key << " failed to initialize properly.";
        }
    }
    LOG(geryon::util::Log::INFO) << "Completed initialization of " << key;
}

void ApplicationModuleContainer::stop() {
    for(auto mp : modules) {
        try {
            mp->stop();
        } catch(std::runtime_error & e) {
            LOG(geryon::util::Log::ERROR) << "Module " << mp->key << " failed to cleanup properly :" << e.what();
        } catch( ... ) {
            LOG(geryon::util::Log::ERROR) << "Module " << mp->key << " failed to cleanup properly.";
        }
    }
    ApplicationModule::stop();
}

bool ApplicationModuleContainer::mustNotifyForSessions() {
    //::TODO:: impr: run only once
    if(ApplicationModule::mustNotifyForSessions()) { return true; }
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) { return true; }
    }
    return false;
}

void ApplicationModuleContainer::notifySessionCreated(Session & ses) {
    ApplicationModule::notifySessionCreated(ses);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionCreated(ses);
        }
    }
}

void ApplicationModuleContainer::notifySessionInvalidated(Session & ses) {
    ApplicationModule::notifySessionInvalidated(ses);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionInvalidated(ses);
        }
    }
}

void ApplicationModuleContainer::notifySessionDestroyed(Session & ses) {
    ApplicationModule::notifySessionDestroyed(ses);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionDestroyed(ses);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueAdded(Session & ses, const std::string & name,
                                                         const boost::any & newVal) {
    ApplicationModule::notifySessionValueAdded(ses, name, newVal);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueAdded(ses, name, newVal);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueChanged(Session & ses, const std::string & name,
                                                           const boost::any & oldVal, const boost::any & newVal) {
    ApplicationModule::notifySessionValueChanged(ses, name, oldVal, newVal);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueChanged(ses, name, oldVal, newVal);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueRemoved(Session & ses, const std::string & name) {
    ApplicationModule::notifySessionValueRemoved(ses, name);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueRemoved(ses, name);
        }
    }
}

void ApplicationModuleContainer::getFilters(std::vector<std::shared_ptr<Filter>> & v) const {
    ApplicationModule::getFilters(v);

    for(auto p : modules) {
        p->getFilters(v);
    }
    LOG(geryon::util::Log::DEBUG) << "Returning " << v.size() << " filters (total).";
}

void ApplicationModuleContainer::getServlets(std::vector<std::shared_ptr<Servlet>> & v) const {
    ApplicationModule::getServlets(v);

    for(auto p : modules) {
        p->getServlets(v);
    }
    LOG(geryon::util::Log::DEBUG) << "Returning " << v.size() << " servlets (total).";
}

} //namespace

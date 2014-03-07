
#include "filter.hpp"
#include "servlet.hpp"
#include "session.hpp"
#include "application.hpp"

#include "log.hpp"

//::TODO:: BIG Q: shall we run in isolation (try-catch) all the listeners and stuff ?

namespace geryon {

/* ====================================================================
 * ApplicationModule
 * ================================================================== */

ApplicationModule::~ApplicationModule() {
    for(auto p : servlets) { delete (p); }
    for(auto p : filters) { delete (p); }
    for(auto p : sessionListeners) { delete (p); }
    for(auto p : moduleListeners) { delete (p); }
}

void ApplicationModule::start() {
    if(status == ApplicationModule::STARTED) {
        return;
    }
    for(auto p : filters) {
        p->init();
    }
    for(auto p : servlets) {
        p->init();
    }
    for(auto p : moduleListeners) {
        p->init();
    }
    status = ApplicationModule::STARTED;
    LOG(geryon::util::Log::INFO) << "Started module " << key;
}

void ApplicationModule::stop() {
    if(status != ApplicationModule::STARTED) {
        return;
    }
    for(auto p : filters) {
        p->done();
    }
    for(auto p : servlets) {
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

void ApplicationModule::notifySessionCreated(Session * const pSes) {
    for(auto p : sessionListeners) {
        p->init(pSes);
    }
}

void ApplicationModule::notifySessionInvalidated(Session * const pSes) {
    for(auto p : sessionListeners) {
        p->invalidated(pSes);
    }
}

void ApplicationModule::notifySessionDestroyed(Session * const pSes) {
    for(auto p : sessionListeners) {
        p->done(pSes);
    }
}

void ApplicationModule::notifySessionValueAdded(Session * const pSes, const std::string & name,
                                                const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->added(pSes, name, newVal);
    }
}

void ApplicationModule::notifySessionValueChanged(Session * const pSes, const std::string & name,
                                                  const boost::any & oldVal, const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->modified(pSes, name, oldVal, newVal);
    }
}

void ApplicationModule::notifySessionValueRemoved(Session * const pSes, const std::string & name) {
    for(auto p : sessionListeners) {
        p->removed(pSes, name);
    }
}

/* ====================================================================
 * ApplicationModuleContainer
 * ================================================================== */

ApplicationModuleContainer::~ApplicationModuleContainer() {
    for(auto mp : modules) {
        delete (mp);
    }
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
    if(ApplicationModule::mustNotifyForSessions()) { return true; }
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) { return true; }
    }
    return false;
}

void ApplicationModuleContainer::notifySessionCreated(Session * const pSes) {
    ApplicationModule::notifySessionCreated(pSes);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionCreated(pSes);
        }
    }
}

void ApplicationModuleContainer::notifySessionInvalidated(Session * const pSes) {
    ApplicationModule::notifySessionInvalidated(pSes);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionInvalidated(pSes);
        }
    }
}

void ApplicationModuleContainer::notifySessionDestroyed(Session * const pSes) {
    ApplicationModule::notifySessionDestroyed(pSes);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionDestroyed(pSes);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueAdded(Session * const pSes, const std::string & name,
                                                         const boost::any & newVal) {
    ApplicationModule::notifySessionValueAdded(pSes, name, newVal);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueAdded(pSes, name, newVal);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueChanged(Session * const pSes, const std::string & name,
                                                           const boost::any & oldVal, const boost::any & newVal) {
    ApplicationModule::notifySessionValueChanged(pSes, name, oldVal, newVal);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueChanged(pSes, name, oldVal, newVal);
        }
    }
}

void ApplicationModuleContainer::notifySessionValueRemoved(Session * const pSes, const std::string & name) {
    ApplicationModule::notifySessionValueRemoved(pSes, name);
    for(auto mp : modules) {
        if(mp->mustNotifyForSessions()) {
            mp->notifySessionValueRemoved(pSes, name);
        }
    }
}

} //namespace

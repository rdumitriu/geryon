
#include "filter.hpp"
#include "servlet.hpp"
#include "session.hpp"
#include "application.hpp"

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
    for(auto p : moduleListeners) {
        p->init();
    }
    status = ApplicationModule::STARTED;
}

void ApplicationModule::stop() {
    for(auto p : moduleListeners) {
        p->done();
    }
    status = ApplicationModule::STOPPED;
}

bool ApplicationModule::mustNotifyForSessions() {
    return !sessionListeners.empty();
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

void ApplicationModule::notifySessionValueAdded(Session * const pSes, const std::string & name, const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->added(pSes, name, newVal);
    }
}

void ApplicationModule::notifySessionValueChanged(Session * const pSes, const std::string & name, const boost::any & oldVal, const boost::any & newVal) {
    for(auto p : sessionListeners) {
        p->modified(pSes, name, oldVal, newVal);
    }
}

void ApplicationModule::notifySessionValueRemoved(Session * const pSes, const std::string & name) {
    for(auto p : sessionListeners) {
        p->removed(pSes, name);
    }
}

} //namespace

#include <thread>
#include <vector>
#include <iostream>

#include "session_listener.hpp"
#include "session.hpp"
#include "application.hpp"

using namespace std;
using namespace geryon;

class _SessionL : public SessionLifecycleListenerAdapter {
public:
    unsigned int _init;
    unsigned int _added;
    unsigned int _modified;
    unsigned int _removed;
    unsigned int _invalidated;
    unsigned int _done;



    _SessionL() : SessionLifecycleListenerAdapter(), _init(0), _added(0), _modified(0),  _removed(0), _invalidated(0), _done(0) {}

    virtual ~_SessionL() {}


    virtual void init(Session * const pSes) {
        _init++;
    }
    virtual void added(Session * const pSes, const std::string & name, const boost::any & val) {
        _added++;
    }
    virtual void modified(Session * const pSes, const std::string & name,
                          const boost::any & oldVal, const boost::any & newVal) {
        _modified++;
    }
    virtual void removed(Session * const pSes, const std::string & name) {
        _removed++;
    }
    virtual void invalidated(Session * const pSes) {
        _invalidated++;
    }
    virtual void done(Session * const pSes) {
        _done++;
    }

};

class _Session : public Session {
public:
    _Session(ApplicationModule * const _pApplication) : Session(_pApplication) {}
    virtual ~_Session() {}
};

class _Application : public Application {
public:
    _Application(const std::string & _key) : Application(_key) {}
    virtual ~_Application() {}

    virtual void notifySessionCreated(Session * const pSes) {
        Application::notifySessionCreated(pSes);
    }
    virtual void notifySessionDestroyed(Session * const pSes) {
        Application::notifySessionDestroyed(pSes);
    }
};

int main(int argn, const char * argv []) {

    _Application app("test.app");
    _SessionL * listener = new _SessionL();
    app.addSessionListener(listener);
    app.start();

    _Session ses(&app);
    app.notifySessionCreated(&ses);

    ses.put("avalue", std::string("buhuhu"));
    ses.put("bvalue", 10);
    std::string valStr;
    if(!ses.get("avalue", valStr)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: avalue should exist (1)" << endl;
    } else if("buhuhu" != valStr) {
        LOG(geryon::util::Log::ERROR) << "FAILED: valStr='" << valStr << "' but it should be 'buhuhu'" << endl;
    }
    int x;
    if(!ses.get("bvalue", x)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: bvalue should exist" << endl;
    } else if( x != 10) {
        LOG(geryon::util::Log::ERROR) << "FAILED: x='" << x << "' but it should be '10'" << endl;
    }
    if(ses.get("cvalue", x)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: cvalue should NOT exist" << endl;
    }

    ses.put("avalue", std::string("bbb"));
    if(!ses.get("avalue", valStr)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: avalue should exist (2)" << endl;
    } else if("bbb" != valStr) {
        LOG(geryon::util::Log::ERROR) << "FAILED: valStr='" << valStr << "' but it should be 'bbb'" << endl;
    }

    ses.remove("bvalue");

    ses.invalidate();


    app.notifySessionDestroyed(&ses);
    app.stop();

    //Now the listener should all be 1
    if(listener->_init != 1 && listener->_added != 1 && listener->_modified != 1 &&
       listener->_removed != 1 && listener->_invalidated != 1 && listener->_done != 1) {
        LOG(geryon::util::Log::ERROR) << "FAILED: Listener values [IAMRID]="
                                      << listener->_init << listener->_added << listener->_modified
                                      << listener->_removed << listener->_invalidated << listener->_done << endl;
    }

    return 0;
}

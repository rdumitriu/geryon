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



    app.notifySessionDestroyed(&ses);
    app.stop();
    return 0;
}

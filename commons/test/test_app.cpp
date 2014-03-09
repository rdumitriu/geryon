#include <thread>
#include <vector>
#include <iostream>

#include "application.hpp"

using namespace std;
using namespace geryon;

class _AppListener : public ApplicationModuleLifecycleListener {
public:
    unsigned int _init;
    unsigned int _done;

    _AppListener() : ApplicationModuleLifecycleListener(), _init(0), _done(0) {}

    virtual ~_AppListener() {}


    virtual void init() {
        _init++;
    }

    virtual void done() {
        _done++;
    }

};

int main(int argn, const char * argv []) {    
    ApplicationModule  * module = new ApplicationModule("module");
    _AppListener * modListener = new _AppListener();
    module->addLifecycleListener(modListener);

    ApplicationModuleContainer app("test.app");
    _AppListener * appListener = new _AppListener();
    app.addLifecycleListener(appListener);
    app.addModule(module);

    app.start();
    if(app.getStatus() != ApplicationModule::STARTED && module->getStatus() != ApplicationModule::STARTED) {
        LOG(geryon::util::Log::ERROR) << "FAILED: app failed to start" << endl;
    }
    app.stop();
    if(app.getStatus() != ApplicationModule::STOPPED && module->getStatus() != ApplicationModule::STOPPED) {
        LOG(geryon::util::Log::ERROR) << "FAILED: app failed to stop" << endl;
    }
    if(modListener->_init != 1 && modListener->_done != 1) {
        LOG(geryon::util::Log::ERROR) << "FAILED: Bad app cycle (module)" << endl;
    }
    if(appListener->_init != 1 && appListener->_done != 1) {
        LOG(geryon::util::Log::ERROR) << "FAILED: Bad app cycle (app)" << endl;
    }
    return 0;
}

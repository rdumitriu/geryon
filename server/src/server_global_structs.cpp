/**
 * \file server_global_structs.cpp
 *
 * Created on: May 07, 2014
 * Author: rdumitriu
 */

#include "server_global_structs.hpp"
#include "os_utils.hpp"

namespace geryon { namespace server {

ServerGlobalStucts ServerGlobalStucts::instance;

ServerGlobalStucts::~ServerGlobalStucts() {
    _clear();
    //now, close the modules
    for(std::vector<void *>::reverse_iterator i = modulesDLLs.rbegin(); i != modulesDLLs.rend(); ++i) {
        geryon::server::closeDynamicLibrary(*i);
    }
    modulesDLLs.clear();
}

//static
void ServerGlobalStucts::clear() {
    instance._clear();
}

void ServerGlobalStucts::_clear() {
    apps.clear();

    std::shared_ptr<geryon::server::GMemoryPool> empty(0);
    memoryPool = empty;
}

//static
void ServerGlobalStucts::setMemoryPool(std::shared_ptr<GMemoryPool> pool) {
    instance.memoryPool = pool;
}

//static
std::shared_ptr<GMemoryPool> ServerGlobalStucts::getMemoryPool() {
    return instance.memoryPool;
}

//static
void ServerGlobalStucts::defineApplication(std::shared_ptr<ServerApplication> app) {
    instance.apps.insert(std::make_pair(app->getPath(), app));
}

//static
std::shared_ptr<ServerApplication> ServerGlobalStucts::getApplication(const std::string & path) {
    std::shared_ptr<ServerApplication> ret;
    std::map<std::string, std::shared_ptr<ServerApplication>>::const_iterator p = instance.apps.find(path);
    if(p != instance.apps.end()) {
        ret = p->second;
    }
    return ret;
}

//static
std::vector<std::shared_ptr<ServerApplication>> ServerGlobalStucts::getApplications() {
    std::vector<std::shared_ptr<ServerApplication>> ret;
    for(auto & p : instance.apps) {
        //just gather them, no matter the order?
        ret.push_back(p.second);
    }
    return ret;
}

//static
void ServerGlobalStucts::setServerToken(const std::string & _serverToken) {
    instance.serverToken = _serverToken;
}

//static
void ServerGlobalStucts::setServerId(unsigned int _serverId) {
    instance.serverId = _serverId;
}

//static
std::string ServerGlobalStucts::getServerToken() {
    return instance.serverToken;
}

//static
unsigned int ServerGlobalStucts::getServerId() {
    return instance.serverId;
}

//static
void ServerGlobalStucts::addModuleDLL(void * ptr) {
    if(ptr) {
        instance.modulesDLLs.push_back(ptr);
    }
}

} } /* namespace */

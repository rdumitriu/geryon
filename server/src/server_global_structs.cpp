/**
 * \file server_global_structs.cpp
 *
 * Created on: May 07, 2014
 * Author: rdumitriu
 */

#include "server_global_structs.hpp"

namespace geryon { namespace server {

ServerGlobalStucts ServerGlobalStucts::instance;

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

} } /* namespace */

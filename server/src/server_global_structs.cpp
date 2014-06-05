/**
 * \file server_global_structs.cpp
 *
 * Created on: May 07, 2014
 * Author: rdumitriu
 */

#include "server_global_structs.hpp"
#include "os_utils.hpp"

namespace geryon { namespace server {

ServerGlobalStructs ServerGlobalStructs::instance;

ServerGlobalStructs::~ServerGlobalStructs() {
    _clear();
    //now, close the modules
    for(std::vector<void *>::reverse_iterator i = modulesDLLs.rbegin(); i != modulesDLLs.rend(); ++i) {
        geryon::server::closeDynamicLibrary(*i);
    }
    modulesDLLs.clear();
}

//static
void ServerGlobalStructs::clear() {
    instance._clear();
}

void ServerGlobalStructs::_clear() {
    apps.clear();

#ifdef G_HAS_PQXX
    postgresPools.clear();
#endif

    std::shared_ptr<geryon::server::GMemoryPool> empty(0);
    memoryPool = empty;
}

//static
void ServerGlobalStructs::setMemoryPool(std::shared_ptr<GMemoryPool> pool) {
    instance.memoryPool = pool;
}

//static
std::shared_ptr<GMemoryPool> ServerGlobalStructs::getMemoryPool() {
    return instance.memoryPool;
}

//static
void ServerGlobalStructs::defineApplication(std::shared_ptr<ServerApplication> app) {
    instance.apps.insert(std::make_pair(app->getPath(), app));
}

//static
std::shared_ptr<ServerApplication> ServerGlobalStructs::getApplication(const std::string & path) {
    std::shared_ptr<ServerApplication> ret;
    std::map<std::string, std::shared_ptr<ServerApplication>>::const_iterator p = instance.apps.find(path);
    if(p != instance.apps.end()) {
        ret = p->second;
    }
    return ret;
}

//static
std::vector<std::shared_ptr<ServerApplication>> ServerGlobalStructs::getApplications() {
    std::vector<std::shared_ptr<ServerApplication>> ret;
    for(auto & p : instance.apps) {
        //just gather them, no matter the order?
        ret.push_back(p.second);
    }
    return ret;
}

//static
void ServerGlobalStructs::setServerToken(const std::string & _serverToken) {
    instance.serverToken = _serverToken;
}

//static
void ServerGlobalStructs::setServerId(unsigned int _serverId) {
    instance.serverId = _serverId;
}

//static
std::string ServerGlobalStructs::getServerToken() {
    return instance.serverToken;
}

//static
unsigned int ServerGlobalStructs::getServerId() {
    return instance.serverId;
}

//static
void ServerGlobalStructs::addModuleDLL(void * ptr) {
    if(ptr) {
        instance.modulesDLLs.push_back(ptr);
    }
}

#ifdef G_HAS_PQXX
//static
void ServerGlobalStructs::addPostgresPool(const std::string & name, std::shared_ptr<PostgresInternalPoolImpl> intp) {
    instance.postgresPools.insert(std::make_pair(name, intp));
}

//static
std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr> ServerGlobalStructs::getPostgresPools() {
    std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr> ret;
    for(auto & s : instance.postgresPools) {
        ret.insert(std::make_pair(s.first, s.second->pool));
    }
    return ret;
}

#endif

} } /* namespace */

/**
 * \file server_global_structs.hpp
 *
 * Created on: May 07, 2014
 * Author: rdumitriu
 */

#ifndef SERVERGLOBALSTRUCTS_HPP_
#define SERVERGLOBALSTRUCTS_HPP_

#include <memory>

#include "mem_buf.hpp"
#include "server_application.hpp"

namespace geryon { namespace server {

///
/// \brief The ServerGlobalStucts class
///
/// Singleton, contains instances of the objects that belong to the whole life-cycle of the server (memory pools,
/// defined applications, etc)
///
/// Not thread safe, so it must be initialized before usage. No operations other than get when doing usual business.
///
class ServerGlobalStucts {
public:
    static void setMemoryPool(std::shared_ptr<GMemoryPool> pool);

    static std::shared_ptr<GMemoryPool> getMemoryPool();

    static void defineApplication(std::shared_ptr<ServerApplication> app);

    static std::shared_ptr<ServerApplication> getApplication(const std::string & path);

    static std::vector<std::shared_ptr<ServerApplication>> getApplications();

    static void setServerToken(const std::string & _serverToken);

    static void setServerId(unsigned int _serverId);

    static std::string getServerToken();

    static unsigned int getServerId();

    static void addModuleDLL(void * ptr);

    static void clear();
private:
    ServerGlobalStucts() {}
    ~ServerGlobalStucts();
    static ServerGlobalStucts instance;

    std::shared_ptr<GMemoryPool> memoryPool;

    std::map<std::string, std::shared_ptr<ServerApplication>> apps;

    std::string serverToken;

    unsigned int serverId;

    //modules
    std::vector<void *> modulesDLLs;

    void _clear();
};

} } /* namespace */


#endif

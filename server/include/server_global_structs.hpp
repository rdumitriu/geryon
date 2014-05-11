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

private:
    ServerGlobalStucts() {}
    ~ServerGlobalStucts() {}
    static ServerGlobalStucts instance;

    std::shared_ptr<GMemoryPool> memoryPool;
};

} } /* namespace */


#endif

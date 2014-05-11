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

} } /* namespace */

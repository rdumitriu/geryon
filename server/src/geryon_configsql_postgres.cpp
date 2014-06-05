
#include <sstream>

#include "server_global_structs.hpp"
#include "geryon_configsql_postgres.hpp"

namespace geryon { namespace server {

void GeryonPostgresConfigurator::setSQLParams(const std::string & dbhost, const std::string & dbport,
                                              const std::string & dbname,
                                              const std::string & user, const std::string & password,
                                              const std::string & dboptions) {
    std::ostringstream connstr;
    if("" == dbhost) {
        throw geryon::sql::SQLException("Postgres configuration: No host specified! Where should I connect ?!?");
    }
    if("" == dbname) {
        throw geryon::sql::SQLException("Postgres configuration: No database specified! Where should I connect ?!?");
    }
    connstr << "host=" << dbhost << " ";
    if("" != dbport) {
        connstr << "port=" << dbport << " ";
    }
    connstr << "dbname=" << dbname << " ";

    if("" != user) {
        connstr << "user=" << user << " "
                << "password=" << password << " ";
    }
    if("" != dboptions) {
        connstr << dboptions;
    }
    connectString = connstr.str();
}


void GeryonPostgresConfigurator::setPoolParams(unsigned int _minSize, unsigned int _maxSize,
                                               unsigned int _connectionTTL, unsigned int _maintenanceInterval,
                                               bool _testOnBorrow, bool _testOnReturn) {
    minSize = _minSize;
    maxSize = _maxSize;
    connectionTTL = _connectionTTL;
    maintenanceInterval = _maintenanceInterval;
    testOnBorrow = _testOnBorrow;
    testOnReturn = _testOnReturn;
}

void GeryonPostgresConfigurator::configure() {
#ifdef G_HAS_PQXX
    ServerGlobalStructs::addPostgresPool(name, std::make_shared<PostgresInternalPoolImpl>(connectString,
                                                                                          minSize, maxSize,
                                                                                          connectionTTL,
                                                                                          maintenanceInterval,
                                                                                          testOnBorrow, testOnReturn));
#endif
}

} } /* namespace */

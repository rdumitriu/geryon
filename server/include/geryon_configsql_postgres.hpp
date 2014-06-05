///
/// \file geryon_configsql_postgres.hpp
///
///  Created on: Jun 05, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYON_POSTGRES_CONFIGURATOR_HPP_
#define GERYON_POSTGRES_CONFIGURATOR_HPP_
#include <string>

namespace geryon { namespace server {

class GeryonPostgresConfigurator {
public:
    GeryonPostgresConfigurator(const std::string & _name) : name(_name) {}

    ~GeryonPostgresConfigurator() {}

    void setSQLParams(const std::string & dbhost, const std::string & dbport,
                      const std::string & dbname,
                      const std::string & user, const std::string & password,
                      const std::string & dboptions);
    void setPoolParams(unsigned int minSize, unsigned int maxSize,
                       unsigned int connectionTTL, unsigned int maintenanceInterval,
                       bool testOnBorrow, bool testOnReturn);

    void configure();
private:
    std::string name;
    std::string connectString;
    unsigned int minSize;
    unsigned int maxSize;
    unsigned int connectionTTL;
    unsigned int maintenanceInterval;
    bool testOnBorrow;
    bool testOnReturn;
};

} }



#endif

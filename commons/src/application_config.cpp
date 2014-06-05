///
/// \file application_config.cpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///

#include "application_config.hpp"

namespace geryon {

///
/// \brief setup the config
/// \param injector the config injector
///
void ApplicationConfig::setup(configuration::ApplicationConfigInjector & injector) {
    mountPath = injector.getMountPath();
    props = injector.getProperties();
    #ifdef G_HAS_PQXX
    postgresConnections = injector.postgresConnections();
    #endif
}

#ifdef G_HAS_PQXX

geryon::sql::postgres::PostgresConnectionPool & ApplicationConfig::getPostgresPool(const std::string & name) {
    std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr>::iterator i = postgresConnections.find(name);
    if(i != postgresConnections.end()) {
        return *(i->second);
    }
    throw geryon::sql::SQLException("SQL resource not defined : " + name);
}

#endif

ApplicationConfig::SQLPoolType ApplicationConfig::getSQLPoolType(const std::string & name) {
#ifdef G_HAS_PQXX
    if(postgresConnections.find(name) != postgresConnections.end()) {
        return SQLPoolType::POSTGRES;
    }
#endif
    return SQLPoolType::NOT_DEFINED;
}

}

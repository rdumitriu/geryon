///
/// \file application_config.hpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_APPLICATION_CONFIG_HPP_
#define GERYON_APPLICATION_CONFIG_HPP_

#include "resources.hpp"

#include "string_utils.hpp"

namespace geryon { namespace configuration {

///
/// \brief The ApplicationConfigInjector class
/// Used to inject the config
///
class G_CLASS_EXPORT ApplicationConfigInjector {
public:
    ApplicationConfigInjector() {}
    virtual ~ApplicationConfigInjector() {}

    ///
    /// \brief Returns the mount path
    /// \return the mount path
    ///
    virtual std::string getMountPath() = 0;

    ///
    /// \brief The application properties to be injected into the configuration
    /// \return the properties map (strings)
    ///
    virtual std::map<std::string, std::string> getProperties() = 0;

#ifdef G_HAS_PQXX
    ///
    /// \brief All the available postgres connections pools
    /// \return the connections pool
    ///
    virtual std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr> postgresConnections() = 0;
#endif
};

}

///
/// \brief The application config
///
/// The application config defines special properties.\n
/// It also defines links to resources
///
class ApplicationConfig {
public:
    ///
    /// \brief The SQLPoolType enum
    ///
    enum SQLPoolType {
        /// Not defined
        NOT_DEFINED,
        /// Postgres
        POSTGRES
    };

    /// Constructor
    explicit ApplicationConfig() {
    }

    ///Destructor
    virtual ~ApplicationConfig() {}

    ///Non-Copyable
    ApplicationConfig(const ApplicationConfig & copy) = delete;

    ///Non-Copyable
    ApplicationConfig & operator =(ApplicationConfig & other) = delete;

    ///
    /// \brief Gets the property
    /// \param key the key
    /// \param defValue the default value
    /// \return the property if found, or empty string if not found
    ///
    inline std::string property(const std::string & key, const std::string & defValue) const {
        std::map<std::string, std::string>::const_iterator p = props.find(key);
        if(p == props.end()) {
            return defValue;
        }
        return p->second;
    }

    ///
    /// \brief Gets a property
    /// \param key the key
    /// \param defValue the default value
    ///
    template<typename T>
    inline T property(const std::string & key, const T & defValue) const {
        std::map<std::string, std::string>::const_iterator p = props.find(key);
        if(p == props.end()) {
            return defValue;
        }
        return geryon::util::convertTo(p->second, defValue);
    }

    ///
    /// \brief Gets the mount path
    ///
    /// \return the path
    ///
    std::string getMountPath() {
        return mountPath;
    }

#ifdef G_HAS_PQXX
    ///
    /// \brief Gets the postgres pool by name
    /// \param name the name
    /// \return the postgres pool, or throws exception if the pool is not defined.
    ///
    geryon::sql::postgres::PostgresConnectionPool & getPostgresPool(const std::string & name);
#endif

    SQLPoolType getSQLPoolType(const std::string & name);

    ///
    /// \brief Setup the config. Called by the application server.
    /// \param injector the config injector
    ///
    void setup(configuration::ApplicationConfigInjector & injector);

private:
    std::string mountPath;
    std::map<std::string, std::string> props;
#ifdef G_HAS_PQXX
    std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr> postgresConnections;
#endif
};

}

#endif

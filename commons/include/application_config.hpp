///
/// \file application_config.hpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef APPLICATION_CONFIG_HPP_
#define APPLICATION_CONFIG_HPP_

#include "string_utils.hpp"

namespace geryon {

namespace configuration {

///
/// \brief The ApplicationConfigInjector class
/// Used to inject the config
class ApplicationConfigInjector {
public:
    ApplicationConfigInjector() {}
    virtual ~ApplicationConfigInjector() {}

    virtual std::string getMountPath() = 0;

    virtual std::map<std::string, std::string> getProperties() = 0;
};

}

///
/// \brief The application config
///
/// The application config defines special properties.\n
/// It also defines links to resources
/// ::TODO:: SQL pools, resources, etc
///
class ApplicationConfig {
public:

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

    ///
    /// \brief setup the config
    /// \param injector the config injector
    ///
    void setup(configuration::ApplicationConfigInjector & injector);

private:
    std::string mountPath;
    std::map<std::string, std::string> props;
};

}

#endif

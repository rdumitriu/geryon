///
/// \file application_config.hpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef APPLICATION_CONFIG_HPP_
#define APPLICATION_CONFIG_HPP_

#include "properties_file.hpp"

namespace geryon {

///
/// \brief The application config
///
/// The application config defines special properties.\n
/// It also defines links to resources
/// ::TODO:: SQL pools, resources, etc
///
class ApplicationConfig {
public:
    static const std::string MOUNT_PATH_PROPERTY;

    /// Constructor
    ApplicationConfig(const std::string & configFile) : props() {
        if(configFile != "") {
            props.loadFromFile(configFile);
        }
    }

    ///Destructor
    virtual ~ApplicationConfig() {}

    ///Non-Copyable
    ApplicationConfig(const ApplicationConfig & copy) = delete;

    ///Non-Copyable
    ApplicationConfig & operator =(ApplicationConfig & other) = delete;


    ///
    /// \brief Gets a property
    /// Gets a certain property by the key. If the property is not found, returns an empty string
    /// \param key the key
    /// \return the value of the property
    ///
    inline std::string property(const std::string & key) const {
        return props.property(key);
    }

    ///
    /// \brief Gets the property
    /// \param key the key
    /// \param defValue the default value
    /// \return the property if found, or empty string if not found
    ///
    inline std::string property(const std::string & key, const std::string & defValue) const {
        return props.property(key, defValue);
    }

    ///
    /// \brief Gets a property
    /// \param key the key
    /// \param defValue the default value
    ///
    template<typename T>
    inline T property(const std::string & key, const T & defValue) const {
        return props.property(key, defValue);
    }

    ///
    /// \brief Gets the mount path
    ///
    /// \return the path
    ///
    std::string getMountPath() {
        return property(MOUNT_PATH_PROPERTY);
    }


private:
    geryon::util::PropertiesFile props;
};

}

#endif

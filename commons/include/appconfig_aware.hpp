///
/// \file appconfig_aware.hpp
///
///  Created on: Dec 08, 2011
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_APPCONFIG_AWARE_HPP_
#define GERYON_APPCONFIG_AWARE_HPP_

#include "platform.hpp"
#include "appexception.hpp"

namespace geryon {

class ApplicationModule;
class ApplicationConfig;

/// Base class to carry over the config
class G_CLASS_EXPORT ApplicationConfigAware {
protected:
    /// Constructor
    ApplicationConfigAware(ApplicationModule * const _pApp = 0) : pApplicationModule(_pApp){}
public:
    ///Destructor
    virtual ~ApplicationConfigAware() {}

    ///Non-Copyable
    ApplicationConfigAware(const ApplicationConfigAware & copy) = delete;

    ///Non-Copyable
    ApplicationConfigAware & operator =(ApplicationConfigAware & other) = delete;

    ///
    /// \brief Gets the application module
    ///
    /// May throw if the object has not been added to some application module. Module will not change during the
    /// life of this object.
    ///
    /// \return the application module \bold where is is defined \bold.
    ///
    ApplicationModule & getApplicationModule() const;


    ///
    /// \brief Returns the configuration for the current module
    /// \return the current config.
    ///
    ApplicationConfig & getModuleConfig() const;

private:
    friend class ApplicationModule;

    inline void setApplicationModule(ApplicationModule * pApp) { this->pApplicationModule = pApp; }

    ApplicationModule * pApplicationModule;
};

}

#endif

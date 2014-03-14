///
/// \file appconfig_aware.hpp
///
///  Created on: Dec 08, 2011
///      Author: rdumitriu at gmail.com
///
#ifndef APPCONFIG_AWARE_HPP_
#define APPCONFIG_AWARE_HPP_

namespace geryon {

class ApplicationModule;

/// Base class to carry over the config
class ApplicationConfigAware {
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
    /// May return NULL if the object has not been added to some application module. This will not change during the
    /// life of this object.
    ///
    /// \return the application module \b where is is defined \b.
    ///
    inline ApplicationModule * getApplicationModule() const { return pApplicationModule; }

private:
    friend class ApplicationModule;

    inline void setApplicationModule(ApplicationModule * pApp) { this->pApplicationModule = pApp; }

    ApplicationModule * pApplicationModule;
};

}

#endif

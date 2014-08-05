///
/// \file application_config.cpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///

#include "application.hpp"
#include "appconfig_aware.hpp"

namespace geryon {

ApplicationConfig & ApplicationConfigAware::getModuleConfig() const {
    if(pApplicationModule) {
        return pApplicationModule->getConfig();
    }
    throw ApplicationException("There's no application module configured. Out of here.");
}

ApplicationModule & ApplicationConfigAware::getApplicationModule() const {
    if(pApplicationModule) {
        return *pApplicationModule;
    }
    throw ApplicationException("There's no application module configured. Out of here.");
}

}


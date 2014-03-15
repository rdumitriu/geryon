///
/// \file application_config.cpp
///
///  Created on: Mar 13, 2014
///      Author: rdumitriu at gmail.com
///

#include "application.hpp"
#include "appconfig_aware.hpp"

namespace geryon {

const ApplicationConfig & ApplicationConfigAware::getModuleConfig() const throw (ApplicationException) {
    if(pApplicationModule) {
        return pApplicationModule->getConfig();
    }
    throw ApplicationException("There's no application module configured. Out of here.");
}

}

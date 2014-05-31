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
}

}


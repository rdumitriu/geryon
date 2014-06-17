///
/// \file os_utils.hpp
///
///  Created on: Dec 08, 2011, retroffited for geryon in Mar 2014
///      Author: rdumitriu at gmail.com
///
#ifndef OSUTILS_HPP_
#define OSUTILS_HPP_

#include <string>

#include "platform.hpp"

#if defined(G_HAS_WIN)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace geryon { namespace server {

///
/// \brief open a dynamic library (.so)
/// \param path the path to file
/// \return the module handler
///
void * openDynamicLibrary(const char * path);

///
/// \brief close a dynamic library (.so)
/// \param moduleHandler the module handler obtained via and \code openDynamicLibrary() \code call
///
void closeDynamicLibrary(void * moduleHandler);

} } /*namespace */

#endif

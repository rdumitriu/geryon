///
/// \file server_application_utils.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef SERVERAPPLICATIONUTILS_HPP_
#define SERVERAPPLICATIONUTILS_HPP_

#include "application.hpp"

namespace geryon { namespace server { namespace detail {

std::string concatPaths(const std::string & first, const std::string & second);

std::string calculatePathFromModule(const ApplicationModule & appm, const std::string &last);

std::string createRegexFromPath(const std::string & path);

} } }

#endif

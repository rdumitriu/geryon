///
/// \file server_application_utils.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_SERVERAPPLICATIONUTILS_HPP_
#define GERYON_SERVERAPPLICATIONUTILS_HPP_

#include <regex>
#include "application.hpp"

namespace geryon { namespace server { namespace detail {

struct MatchingEntry {
    std::string mappedPath;
    unsigned int noStars;
    std::string beginOfPath;
    std::string endOfPath;
    std::regex regex;
};

std::string concatPaths(const std::string & first, const std::string & second);

std::string calculatePathFromModule(const ApplicationModule & appm, const std::string &last);

//std::string createRegexFromPath(const std::string & path);

MatchingEntry createMatchingEntry(const std::string & path);

bool isMatchingEntry(const std::string & path, const MatchingEntry & mentry);

} } }

#endif

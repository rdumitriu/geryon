/**
 * \file conversions.hpp
 *
 * Created on: Feb 25, 2014
 * Author: rdumitriu at gmail.com
 */
#ifndef CONVERSIONS_HPP_
#define CONVERSIONS_HPP_

#include <sstream>
#include <string>
#include <chrono>

#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>

namespace geryon { namespace util {

///
/// \brief Different conversions, to hide out our dependency on boost
/// (where we can)
///


///
/// \brief convert a string to a basic type
///
/// \param s the string
/// \param defaultValue the default value, optional
///
template <typename T>
inline T convertTo(const std::string & s, T defaultValue) {
    try {
        if(!s.empty()) {
            return boost::lexical_cast<T>(s);
        }
    }
    catch(const boost::bad_lexical_cast &) {} //we do not care
    return defaultValue;
}

///
/// \brief Convert a date-time
///
/// Converts an ISO date time, from yyyy-MMM-dd hh:mm:ss format to time_t
///
/// \param s the string, in the form yyyy-MMM-dd hh:mm:ss
/// \return the time, or a zero initialized structure if it fails
///
inline std::time_t convertISODateTime(const std::string & s) {
    try {
        boost::posix_time::ptime pt;
        std::stringstream ss(s);
        ss.exceptions(std::ios_base::failbit);
        ss >> pt;
        //we have now the posix time
        boost::posix_time::time_duration::sec_type secs = (pt - boost::posix_time::from_time_t(0)).total_seconds();
        return std::time_t(secs);
    } catch( ... ) {} //we do not care
    return std::time_t(0UL);
}

///
/// \brief Formats an time_t to an ISO representation
///
/// \param t the time_t struct
/// \return the textual representation of it
///
inline std::string formatISODateTime(const std::time_t & t) {
    std::stringstream ss;
    boost::posix_time::ptime pt = boost::posix_time::from_time_t(t);
    ss << pt;
    return ss.str();
}

///
/// \brief startsWith
/// \param s the string
/// \param begin the string to be searched
/// \return true if the
///
inline bool startsWith(const std::string & s, const std::string & begin) {
    return boost::starts_with(s, begin);
}

///
/// \brief Trim a string on both sides
///
/// In place trim.
///
/// \param s the string
///
inline void trim(std::string & s) {
    boost::trim(s);
}

} }

#endif //LOG_HPP_

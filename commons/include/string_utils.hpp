/**
 * \file string_utils.hpp
 *
 * Created on: Feb 25, 2014
 * Author: rdumitriu at gmail.com
 */
#ifndef GERYON_STRING_UTILS_HPP_
#define GERYON_STRING_UTILS_HPP_

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
/// \return true if the string starts with the second param
///
inline bool startsWith(const std::string & s, const std::string & begin) {
    return boost::starts_with(s, begin);
}

///
/// \brief toUpper
/// \param s the string
///
inline void toUpper(std::string & s) {
    boost::to_upper(s);
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

///
/// \brief Trim a string on both sides
///
/// In place trim.
///
/// \param s the string
///
inline std::string trimAndCopy(const std::string & s) {
    return boost::trim_copy(s);
}

/* ===================================================================
 * H T T P  S U P P O R T
 * =================================================================== */

inline bool isHTTPChar(char c) {
    return c >= 0 && c <= 127;
}

inline bool isHTTPCtl(char c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

inline bool isHTTPDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isHTTPSpecial(char c);

bool decodeURL(const std::string& in, std::string& out);

void encodeURL(const std::string& in, std::string& out);

} }

#endif //LOG_HPP_

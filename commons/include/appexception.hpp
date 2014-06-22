///
/// \file appexception.hpp
///
/// Created on: Mar 23, 2014
/// Author: rdumitriu at gmail.com
///
#ifndef GERYON_APP_EXCEPTION_HPP_
#define GERYON_APP_EXCEPTION_HPP_
 
#include <string>

namespace geryon {

///
/// \brief The application exception
///
class ApplicationException : public std::runtime_error {
public:
    ///
    /// \brief Constructor.
    ///
    /// \param msg the error message
    ///
    explicit ApplicationException(const std::string & msg) : std::runtime_error(msg) {}

    ///
    /// \brief Destructor
    ///
    virtual ~ApplicationException() throw() {}
};

}

#endif

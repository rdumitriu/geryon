///
/// \file log.hpp
///
/// Idea by mr. Petru Marginean, published in DrDobbs Journal (OCT07).
/// \todo log rotation, thread on log
///
/// Created on: Mar 5, 2011
/// Author: rdumitriu at gmail.com
///
#ifndef GERYON_LOG_HPP_
#define GERYON_LOG_HPP_

#include <sstream>
#include <string>

namespace geryon { namespace util {

/// \brief The log class
///
/// The log is very simple but efficient. For now, it does not care about the
/// log rotation. Please call just once one of the configure routine.
///
/// \todo 1/ Log should have multiple sinks.\n
/// 2/ The idea is to split the log into access_log, server_log, application_log * N (for each application a log) \n
/// 3/ log rotation \n
/// 4/ Should measure performance and check if not the actual log should be a thread, taking log messages from a queue.
/// The idea would be NOT to write files in the thread actually doing the work, since it's detriemental to perf.

class Log {
public:
    ///
    /// The log level
    ///
	enum LogLevel { FATAL, ERROR, WARNING, INFO, DEBUG };

    ///
    /// Default constructor
    ///
    Log();

    ///
    /// Destructor
    ///
    virtual ~Log();
    
    /// Non-copyable
    Log(const Log & copy) = delete;
    /// Non-copyable
    Log & operator = (const Log & other) = delete;

    ///
    /// Gets the stream to write to.
    /// \param level the log level
    ///
    std::ostringstream& get(LogLevel level = INFO);


    ///
    /// \brief Simple configuration : no log.
    /// No logging is performed.
    ///
    static void configureNone();
    
    ///
    /// \brief Configure log to stderr, desired level.
    /// Log is performed on stderr, level is provided
    /// \param level the log level
    ///
    static void configureBasic(LogLevel level);
    
    ///
    /// \brief Configure logging into a file, level is the desired one.
    /// Log is performed into a file, level is the one provided
    /// \param level the log level
    /// \param fileName the file name
    ///
    static void configureFile(LogLevel level,
    						  const std::string & fileName);

    ///
    /// \brief Gets the reporting level.
    /// \return the log level
    ///
    static const LogLevel getReportingLevel();
    
    ///
    /// Transforms the level to a human readable string.
    /// \param level the log level
    /// \return the string representing that level
    ///
    static std::string toString(const LogLevel level);
    
    ///
    /// Transforms the level to a human readable string.
    /// \param level the log level as a string
    /// \return the internal representation of that level
    ///
    static LogLevel fromString(const std::string& level);
protected:
    ///the output stream where we build it all up
    std::ostringstream os;
private:

    std::string currentTimestampAsString();
};

///
///\def LOG_MAX_LEVEL
/// Defining LOG_MAX_LEVEL on INFO will eliminate at compile time all the
/// messages under the INFO level
///

#ifndef LOG_MAX_LEVEL
#define LOG_MAX_LEVEL geryon::util::Log::DEBUG
#endif

///
/// \def LOG(level)
/// Use LOG macro to log messages.
///

#ifndef LOG
#define LOG(level) \
    if ( level > LOG_MAX_LEVEL || level > geryon::util::Log::getReportingLevel() ) ; \
    else geryon::util::Log().get(level)
#endif

}
}

#endif //LOG_HPP_

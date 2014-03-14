/*
 * log.cpp
 *
 *  Created on: Mar 10, 2011
 *      Author: rdumitriu at gmail dot com
 */
#include <stdio.h>
#include <string>
#include <sstream>
#include <cstdlib>

#include "string_utils.hpp"

#include "log.hpp"


namespace geryon { namespace util {

static const char* const
logLevelNames[] = {"FATAL", "ERROR", "WARNING", "INFO", "DEBUG" };

static FILE * sink = stderr;
static Log::LogLevel logLevel = Log::INFO;



void closeSink() {
	if(sink && sink != stderr) {
		fclose(sink);
		sink = stderr;
	}
}

/* -------------------------------------------------------------------------
 * LOG IMPLEMENTATION
 * ------------------------------------------------------------------------- */

Log::Log() : os() {
}

Log::~Log() {
	os << std::endl;
	if (!sink) {
		return;
	}
	fprintf(sink, "%s", os.str().c_str());
	fflush(sink);
}

std::ostringstream& Log::get(LogLevel level) {
	os << "[" << toString(level) << "] " << currentTimestampAsString() << " :";
	return os;
}


void Log::configureNone() {
    sink = 0;
	logLevel = FATAL;
}

void Log::configureBasic(LogLevel level) {
	sink = stderr;
	logLevel = level;
}

void Log::configureFile(LogLevel level,
						const std::string & fileName) {
	FILE * f = fopen(fileName.c_str(), "a");
	if(!f) {
		fprintf(stderr, "Cannot open LOG file >>%s<<. Falling back to stderr.",
						fileName.c_str());
		configureBasic(level);
		return;
	}
	atexit(&closeSink);
	sink = f;
	logLevel = level;
}

/*
void Log::configureFromFile(const std::string & propsFile) {
	Properties pfile(propsFile);
	std::string disableStr = pfile.property("log.disable");
	boost::trim(disableStr);
	bool disable = (disableStr == "1" || disableStr == "true" ||
                    disableStr == "T" || disableStr == "t");
	if(disable) {
		configureNone();
	} else {
        std::string levelStr = pfile.property("log.level");
        Log::LogLevel level = fromString(levelStr);
        std::string file = pfile.property("log.file");
        if(file == "") {
            configureBasic(level);
        } else {
            configureFile(level, file);
        }
    }
}
*/

const Log::LogLevel Log::getReportingLevel() {
	return logLevel;
}

std::string Log::toString(const LogLevel level) {
	return std::string(logLevelNames[level]);
}

Log::LogLevel Log::fromString(const std::string & level) {
	for(int i = 0 ; i < 5; i++) {
		if(level == logLevelNames[i]) {
			return static_cast<LogLevel>(i);
		}
	}
	fprintf(stderr, "Cannot understand log level >>%s<<. Logging level reset to INFO.",
					level.c_str());
	return INFO;
}

std::string Log::currentTimestampAsString() {
	//use microseconds clock
	std::ostringstream ss;
	//serialize it
    std::chrono::time_point<std::chrono::system_clock> curr = std::chrono::system_clock::now();
    std::time_t curr_t = std::chrono::system_clock::to_time_t(curr);
    return formatISODateTime(curr_t);
}

}
}

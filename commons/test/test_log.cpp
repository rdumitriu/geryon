/*
 * test_log.cpp
 *
 *  Created on: Mar 11, 2011
 *      Author: rdumitriu
 */
#include <iostream>

#include "log.hpp"

using namespace std;
using namespace geryon::util;

void testLevels() {
	Log::configureBasic(Log::INFO);
	LOG(Log::INFO) << "[1] Log works [INFO]?";
	LOG(Log::ERROR) << "[1] Log works [ERROR]?";
	LOG(Log::DEBUG) << "[1] FAILED: This log should not appear in the log";

	Log::configureBasic(Log::FATAL);
	LOG(Log::INFO) << "[2] FAILED: Should not appear [INFO]?";
	LOG(Log::ERROR) << "[2] FAILED: Should not appear [ERROR]?";
	LOG(Log::DEBUG) << "[2] FAILED: Should not appear [DEBUG]?";
}

void testFile() {
	Log::configureFile(Log::INFO, "application.log");
	LOG(Log::INFO) << "[1] Log works [INFO]?";
	LOG(Log::ERROR) << "[1] Log works [ERROR]?";
	LOG(Log::DEBUG) << "[1] FAILED: This log should not appear in the log";
}

int main(int argn, const char * argv []) {
	testLevels();
	testFile();

	return 0;
}

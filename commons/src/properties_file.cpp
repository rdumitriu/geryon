/*
 * \file properties_file.cpp
 *
 *  Created on: Mar 22, 2011, changed on 13 Mar 2014
 *      Author: rdumitriu
 */
#include <string>
#include <map>

#include <iostream>
#include <fstream>

#include "string_utils.hpp"
#include "properties_file.hpp"

#include "log.hpp"

namespace geryon { namespace util {

PropertiesFile::PropertiesFile(const std::string & file) {
	loadFromFile(file);
}

const bool PropertiesFile::hasProperty(const std::string & key) const {
    std::map<std::string, std::string>::const_iterator p = props.find(key);
    return (p != props.end());
}

const std::string PropertiesFile::property(const std::string & key) const {
	std::map<std::string, std::string>::const_iterator p = props.find(key);
	if(p == props.end()) {
        return "";
	}
	return p->second;
}

const std::string PropertiesFile::property(const std::string & key, const std::string & defValue) const {
    std::map<std::string, std::string>::const_iterator p = props.find(key);
    if(p == props.end()) {
        return defValue;
    }
    return p->second;
}

void PropertiesFile::loadFromFile(const std::string & file) {
	std::string line;
	std::ifstream stream;

	stream.open(file.c_str());
	if(!stream.is_open()) {
		LOG(Log::ERROR) << "Cannot open properties file :" << file;
		return;
	}
	int lineNo = 0;
	while(!stream.eof()) {
		std::getline (stream, line);
		lineNo++;
        trim(line);
		if(line.length() == 0) { continue; }
		if(line[0] == '#') { //this is a comment
			continue;
		}
		std::size_t ndx = line.find('=');
		if(ndx == std::string::npos || ndx == 0) {
			LOG(Log::WARNING) << "Cannot understand line no " << lineNo << ". Skipping over[1]:" << line;
			continue;
		}
		std::string key = line.substr(0, ndx);
		std::string value = (ndx < line.length() - 1) ? line.substr(ndx+1) : "";
		LOG(Log::DEBUG) << "Properties file[" << file << ":" << lineNo << "] Key=" << key << " value=" << value;
        trim(key);
        trim(value);
		if(key == "") {
			LOG(Log::WARNING) << "Cannot understand line no " << lineNo << ". Skipping over [2]:" << line;
			continue;
		}
		props.insert(std::make_pair(key, value));
	}
	stream.close();
}

} } /*namespace */

/*
 * TestPropertiesFile.C
 *
 *  Created on: Mar 22, 2011
 *      Author: rdumitriu
 */
#include <string>

#include "properties_file.hpp"
#include "log.hpp"

using namespace std;
using namespace geryon::util;

int main(int argn, char * argv []) {
	if(argn != 2) {
		LOG(Log::ERROR) << "You should provide a file (properties)";
		return -1;
	}

    PropertiesFile p(argv[1]);

	string s = p.property("property_1");
	if(s != "The First Property") {
        LOG(Log::ERROR) << "FAILED : property_1=" << s << "|" << endl;
	}
	s = p.property("property_2");
	if(s != "The  Second  Property") {
        LOG(Log::ERROR) << "FAILED : property_2=" << s << "|" << endl;
	}
	s = p.property("property 3");
	if(s != "The Third Property has spaces") {
        LOG(Log::ERROR) << "FAILED : property 3=" << s << "|" << endl;
	}
	s = p.property("property_4");
	if(s != "") {
        LOG(Log::ERROR) << "FAILED : property_4=" << s << "|" << endl;
	}
    unsigned int x = p.property("int.1", 0u);
    int y = p.property("int.2", 0);
    if(x != 1234 || y != -23) {
        LOG(Log::ERROR) << "FAILED : int x=" << x << "| y=" << y << endl;
    }
	return 0;
}

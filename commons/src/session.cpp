/*
 * \file session.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: rdumitriu at gmail dot com
 */
#include "session.hpp"
#include "log.hpp"

namespace geryon {

void Session::notifySessionValueAdded(const std::string & name, const boost::any & val) {
    if(getApplicationModule().mustNotifyForSessions()) {
        getApplicationModule().notifySessionValueAdded(*this, name, val);
    }
}

void Session::notifySessionValueChanged(const std::string & name, const boost::any & oldval, const boost::any & newval) {
    if(getApplicationModule().mustNotifyForSessions()) {
        getApplicationModule().notifySessionValueChanged(*this, name, oldval, newval);
    }
}

void Session::notifySessionValueRemoved(const std::string & name) {
    if(getApplicationModule().mustNotifyForSessions()) {
        getApplicationModule().notifySessionValueRemoved(*this, name);
    }
}

void Session::notifySessionInvalidated() {
    if(getApplicationModule().mustNotifyForSessions()) {
        getApplicationModule().notifySessionInvalidated(*this);
    }
}

}

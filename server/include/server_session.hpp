///
/// \file server_session.hpp
///
///  Created on: May 24, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_SERVERSESSION_HPP_
#define GERYON_SERVERSESSION_HPP_

#include "session.hpp"

namespace geryon { namespace server {

class ServerSession : public Session {
public:
    explicit ServerSession(ApplicationModule * const _pApplication) : geryon::Session(_pApplication) {}
    virtual ~ServerSession() {}

    inline void updateTimeStamp() {
        std::lock_guard<std::mutex> _(mutex);
        updateTimeStampNoLock();
    }

    inline std::time_t getTimeStamp() {
        std::lock_guard<std::mutex> _(mutex);
        return timeStamp;
    }
};

} }

#endif

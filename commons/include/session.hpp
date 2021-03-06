///
/// \file session.hpp
///
///  Created on: Dec 08, 2011, re-worked on 1st of March, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_SESSION_HPP_
#define GERYON_SESSION_HPP_
 
#include <string>
#include <map>
#include <chrono>
#include <mutex>

#include "http_types.hpp"
#include "appconfig_aware.hpp"
#include "application.hpp"

namespace geryon {

///
/// \brief The session
///
/// All objects placed in the session are owned by the session. When destroyed, it will destroy objects too. \n
/// So it's ok to do
/// @code
/// session.put("object-key", std::move(x));
/// long lx = 1;
/// if(!session.get("my-key", lx)) { ... }
/// @endcode
/// ... where x is a copyable object.\n\n
/// The session is thread safe, but values stored inside are NOT! You have to remember that multiple threads may
/// access the same session. Session use internally boost::any to care for your values.
/// @todo Design an interf to offer no-lock semantics (mutex is already available, but it is not re-entrant, so ...)
///
class G_CLASS_EXPORT Session : public ApplicationConfigAware {
protected:
    ///
    /// \brief Constructor
    ///
    /// You cannot construct a session directly.
    ///
    /// \param _pApplication the pointer to the application object
    ///
    explicit Session(ApplicationModule * const _pApplication)
            : ApplicationConfigAware(_pApplication),
              timeStamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}
public:
    ///
    /// \brief Destructor.
    ///
    /// Clears all the session values
    ///
    virtual ~Session() {}

    ///Non-copyable
    Session(const Session & copy) = delete;
    ///Non-copyable
    Session & operator= (const Session & other) = delete;

    ///
    /// \brief Returns a reference to the underlying mutex.
    /// It can be used to lock the session for further modifications
    /// \return the reference to the mutex
    std::mutex & getMutex() { return mutex; }
    
    ///
    /// \brief Puts values in the session.
    ///
    /// Puts objects in the session; objects should accept copy semantics (copy constructor).\n\n
    /// Locks the current session while updating. Therefore, at this boundary you are isolated from other changes;
    /// however, the values are not.
    ///
    /// \param name the name of the value
    /// \param obj the value
    ///
    template <typename T>
    void put(const std::string & name, const T & obj) {
        bool upd = false;
        boost::any oldObj;
        {
            std::lock_guard<std::mutex> _(mutex);
            updateTimeStampNoLock();
            auto p = attributes.find(name);
            if(p != attributes.end()) {
                upd = true;
                oldObj = p->second;
                attributes.erase(p);
            }
            attributes.insert(std::make_pair(name, obj));
        }
        // Free from locks!
        if(upd) {
            notifySessionValueChanged(name, oldObj, obj);
        } else {
            notifySessionValueAdded(name, obj);
        }
    }

    ///
    /// \brief Gets value from the session.
    ///
    /// This version gets objects from the session, but these objects should accept copy semantics (copy
    /// constructor).\n\n
    /// Locks the current session while getting the value. Therefore, at this boundary you are isolated from other
    /// changes; however, the values are not. After return, anybody may alter that value from session.
    ///
    /// \param name the name of the value
    /// \param obj the object to store the value
    /// \return true if the object was found in session
    ///
    template <typename T>
    bool get(const std::string & name, T & obj) {
        try {
            std::lock_guard<std::mutex> _(mutex);
            updateTimeStampNoLock();
            auto p = attributes.find(name);
            if(p != attributes.end()) {
                obj = boost::any_cast<T>(p->second);
                return true;
            }
        } catch(boost::bad_any_cast & e) {
            std::string msg = "Bad cast getting session attribute:" + name;
            LOG(geryon::util::Log::ERROR) << msg;
        }
        return false;
    }
    
    ///
    /// \brief Removes a named variable from the session.
    ///
    /// \param name the name
    ///
    bool remove(const std::string & name) {
        bool ret = false;
        {
            std::lock_guard<std::mutex> _(mutex);
            updateTimeStampNoLock();
            auto p = attributes.find(name);
            if(p != attributes.end()) {
                attributes.erase(p);
                ret = true;
            }
        }
        // Free from locks!
        if(ret) {
            notifySessionValueRemoved(name);
        }
        return ret;
    }

    ///
    /// \brief Invalidates a session
    ///
    /// When you're done, you may request that all the values from this session to be scraped. Session itself it is
    /// not destroyed. If the user will come back before the time, the same session will be reused.
    ///
    inline void invalidate() {
        {
            std::lock_guard<std::mutex> _(mutex);
            updateTimeStampNoLock();
            attributes.clear();
        }
        // Free from locks !
        notifySessionInvalidated();
    }
    
protected:
    std::mutex mutex;
    std::time_t timeStamp;
    std::map<std::string, boost::any> attributes;

    inline void updateTimeStampNoLock() {
        timeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
private:
    void notifySessionValueAdded(const std::string & name, const boost::any & val);
    void notifySessionValueChanged(const std::string & name, const boost::any & oldval, const boost::any & newval);
    void notifySessionInvalidated();
    void notifySessionValueRemoved(const std::string & name);
};

}

#endif

/*
 * Session.hpp
 *
 *  Created on: Dec 08, 2011
 *      Author: rdumitriu
 */
#ifndef SESSION_HPP_
#define SESSION_HPP_
 
#include <string>
#include <map>

#include <HTTPTypes.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace agrade { namespace yahsrv {

//FWDs
class Application;
class Session;

/**
 * \brief Signals the start/end/modify of a session
 * 
 * Such a listener should be configured before the application starts. \n
 * Code MUST be thread safe. \n\n
 * We prefer to keep listeners as classes with behavior instead of having 
 * pointers to functions for that matter.
 */
class SessionLifecycleListener {
public:
    /**
     * \brief Constructor
     */
    SessionLifecycleListener() {}
    /**
     * \brief Destructor
     */
    virtual ~SessionLifecycleListener() {}
    
    /**
     * \brief Called at initialization time 
     * 
     * \param app the application
     * \param ses the session
     */
    virtual void init(Application & app, Session & ses) = 0;
    /**
     * \brief Called when a values is added on a session
     * 
     * You will need to cast the data to your real data (of type SessionValue<T>)
     * 
     * \param app the application
     * \param ses the session
     * \param name the name of the variable
     * \param pData the data
     */
    virtual void added(Application & app, 
                       Session & ses, 
                       const std::string & name,
                       const ValueHolder * pData) = 0;
    /**
     * \brief Called when a values is altered on a session
     * 
     * You will need to cast the data to your real data (of type SessionValue<T>)
     * 
     * \param app the application
     * \param ses the session
     * \param name the name of the variable
     * \param pData the data
     */
    virtual void modified(Application & app, 
                          Session & ses, 
                          const std::string & name,
                          const ValueHolder * pData) = 0;
    /**
     * \brief Called when a values is removed on a session
     * 
     * \param app the application
     * \param ses the session
     * \param name the name of the variable
     */
    virtual void removed(Application & app,
                         Session & ses,
                         const std::string & name) = 0;
                         
    /**
     * \brief Called when we're done with the session
     * 
     * \param app the application
     * \param ses the session
     */
    virtual void done(Application &app, Session & ses) = 0;
};


/**
 * \brief The session
 * 
 * All objects placed in the session are owned by the session. When destroyed,
 * it will destroy objects too. \n
 * So it's ok to do
 * @code
 * session.put<std::string>("object-key", new std::string("bla bla"));
 * @endcode
 * ... and forget about it.\n\n
 * The session is thread safe, but values stored inside are NOT! You have to 
 * remember that multiple threads may access the same session.
 */
class Session {
private:
    boost::posix_time::ptime m_timeStamp;
    boost::shared_mutex m_mutex;
    Application * m_pApplication;
    std::map<std::string, ValueHolder *> m_objects;
public:
    /**
     * \brief Constructor
     * 
     * \param pApplication the pointer to the application object
     */
    explicit Session(Application * pApplication);
    /**
     * \brief Destructor.
     * 
     * Clears all the session values
     */
    virtual ~Session();
    
    /**
     * \brief Puts values in the session.
     * 
     * Locks the current session while updating. Therefore, at this boundary you
     * are isolated from other changes; however, the values are not.
     * 
     * \param name the name of the value
     * \param obj the value
     */
    template <typename T>
    std::shared_ptr<T> put(const std::string & name, T * obj) {
        boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
        boost::upgrade_to_unique_lock<boost::shared_mutex> lock_excl(lock);
    
        std::map<std::string, ValueHolder *>::iterator p = m_objects.find(name);
        if(p != m_objects.end()) {
            TPValueHolder<T> *pSV = static_cast<TPValueHolder<T> *>(p->second);
            pSV->set(obj);
            notifyValueUpdated(name, p->second);
            return pSV->get();
        } else {
            TPValueHolder<T> *pData = new TPValueHolder<T>(obj);
            m_objects.insert(std::make_pair(name, pData));
            notifyValueAdded(name, pData); //if this fails, we still have the reference
            return pData->get();
        }
    }
    
    /**
     * \brief Puts values in the session.
     * 
     * This version puts objects in the session, but objects should accept copy 
     * semantics (copy constructor).\n\n
     * Locks the current session while updating. Therefore, at this boundary you
     * are isolated from other changes; however, the values are not.
     * 
     * \param name the name of the value
     * \param obj the value
     */
    template <typename T>
    std::shared_ptr<T> put(const std::string & name, const T & obj) {
        T * pT = NULL;
        try {
            T * pT = new T(obj);
            return put(name, pT);
        } catch( ... ) {
            delete pT;
            throw;
        }
    }

    /**
     * \brief Gets value from the session.
     * 
     * This version puts objects in the session, but objects should accept copy 
     * semantics (copy constructor).\n\n
     * Locks the current session while getting the value. Therefore, at this 
     * boundary you are isolated from other changes; however, the values are not.
     * After return, anybody may alter that value.
     * 
     * \param name the name of the value
     * \return the value, wrapped in a shared pointer
     */
    template <typename T>
    std::shared_ptr<T> get(const std::string & name) {
        boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    
        std::map<std::string, ValueHolder *>::iterator p = m_objects.find(name);
        if(p != m_objects.end()) {
            return (static_cast<TPValueHolder<T> *>(p->second))->get();
        }
        std::shared_ptr<T> ret(static_cast<T *>(NULL));
        return ret;
    }
    
    /**
     * \brief Removes a named variable from the session.
     * 
     * \param name the name
     */
    void remove(const std::string & name);
    
    /**
     * \brief Time-Stamp of the session
     * 
     * It is used to evict the session
     * 
     * @return the time stamp of the session
     */
    boost::posix_time::ptime getTimeStamp();
    
    /**
     * \brief Updates the Time-Stamp of the session
     * 
     * It is used to avoid to evict the session
     * 
     * @return the time stamp of the session
     */
    void updateTimeStamp();
    
    /**
     * \brief Invalidates a session
     * 
     * When you're done, you may request that all the values from this session
     * to be scraped. Session itself it is not destroyed. If the user will come
     * back before the, the same session will be reused.
     */
     void invalidate();
    
private:
    void notifyValueAdded(const std::string & name, ValueHolder *pData);
    void notifyValueUpdated(const std::string & name, ValueHolder *pData);
    void notifyValueRemoved(const std::string & name);
};

} } 

#endif

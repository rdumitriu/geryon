///
/// \file monitor_queue.hpp
///
/// Created on: Feb 17, 2014
/// Author: rdumitriu at gmail.com
///
#ifndef GERYON_MONITOR_QUEUE_HPP_
#define GERYON_MONITOR_QUEUE_HPP_

#include <mutex>
#include <condition_variable>

#include "platform.hpp"

namespace geryon { namespace mt {

///
/// \brief A monitor queue
///
/// The buffer queue is basis of any monitor. The type T must be copyable.\n\n
/// Destruction of the queue will destroy all objects in it, so make sure the processing is complete.
///
template <typename T>
class G_CLASS_EXPORT MonitorQueue {
public:

    ///Constructor.
    explicit
    MonitorQueue(std::size_t n = 0) : maxSz(n),
                                 p(0), c(0), sz(0),
                                 buffer(new T[n]) {
    }

    ///Destructor
    ~MonitorQueue() {
        delete [] buffer;
    }

    ///
    /// \brief put into queue
    /// \param obj the copyable object to be placed in queue.
    ///
    /// Puts something in our buffer. Blocks if not enough space
    ///
    void put(T obj) {
        std::unique_lock<std::mutex> lock(mutex);

        while (sz == maxSz) {
            cond.wait(lock);
        }
        buffer[p] = obj;
        p = (p+1) % maxSz;
        ++sz;

        //lock.unlock(); //manual unlocking here
        cond.notify_one();
    }

    ///
    /// \brief Gets an copyable object from the queue.
    /// \return the object
    ///
    ///  Gets something from the buffer. Blocks if nothing is in there
    ///
    T get() {
        std::unique_lock<std::mutex> lock(mutex);
        while (sz == 0) {
            cond.wait(lock);
        }
        T ret = buffer[c];
        c = (c+1) % maxSz;
        --sz;

        cond.notify_one();
        return ret;
    }

    ///Gets the size of the pool, as it is
    std::size_t size() {
        std::unique_lock<std::mutex> lock(mutex);
        return sz;
    }

    ///Gets the max size
    std::size_t maxSize() {
        return maxSz;
    }

    /// Non-copyable
    MonitorQueue(const MonitorQueue & copy) = delete;
    /// Non-copyable
    MonitorQueue & operator = (const MonitorQueue & other) = delete;

private:

    std::condition_variable cond; /*condition */
    std::mutex mutex; /* the mutex */
    std::size_t maxSz; /* how many elements are allowed */
    std::size_t p; /* producer pointer */
    std::size_t c; /* consumer pointer */
    std::size_t sz; /* size of the pool */
    T *buffer;
};

} } /* namespace */
#endif

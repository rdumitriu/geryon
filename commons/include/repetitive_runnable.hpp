/**
 * \file repetitive_runnable.hpp
 *
 * Created on: Feb 18, 2014
 * Author: rdumitriu at gmail.com
 */
#ifndef REPETITIVE_RUNNABLE_HPP_
#define REPETITIVE_RUNNABLE_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>

namespace geryon { namespace mt {

///
/// \brief The RepetitiveRunnable class.
///
/// It sleeps for a number of seconds; no result is returned.
/// You need to call start() for this to run your task. Calling stop() is optional, as it will be
/// called by destructor if needed.
///
class RepetitiveRunnable {
public:
    ///
    /// \brief Constructor
    /// \param _sleepTime the sleep time for each cycle
    /// \param _c the function or the object instance to be run
    ///
    template<typename _Callable, typename... _Args>
    explicit RepetitiveRunnable(u_int32_t _sleepTime, _Callable _c, _Args ..._args)
                    : sleepTime(_sleepTime), mustStop(false),
                      f(std::bind(_c, _args...)), p_t(NULL) {
    }

    ///
    /// \brief Copy constructor
    /// \param copy the object to be copied.
    /// It only copies the parameters, never the the dynamic aspects
    ///
    explicit RepetitiveRunnable(const RepetitiveRunnable & copy)
                    : sleepTime(copy.sleepTime), mustStop(false), f(copy.f), p_t(NULL) {
    }

    ///
    /// \brief operator =
    /// \param other the object on the other end
    /// \return the reference to this
    /// It only copies the parameters, never the the dynamic aspects
    ///
    RepetitiveRunnable & operator =(const RepetitiveRunnable & other) {
        if(this != &other) {
            this->sleepTime = other.sleepTime;
            this->mustStop = false;
            this->f = other.f;
        }
        return (*this);
    }

    ///
    /// \brief Call this to start repetitive execution
    ///
    void start() {
        try {
            p_t = new std::thread([this](){
                while(!mustStop) {
                    using namespace std::chrono;
                    std::unique_lock<std::mutex> _(mutex);
                    system_clock::time_point s = system_clock::now();

                    while(!mustStop) {
                        seconds d = seconds(sleepTime) - seconds(duration_cast<seconds>(system_clock::now() - s));
                        if(d.count() > 0) {
                            cond.wait_for(_, d);
                        } else {
                            break;
                        }
                    }
                    if(!mustStop) {
                        try {
                            f();
                        } catch( ... ) {
                            //::TODO:: errors !!!
                        }
                    }
                }
            });
        } catch( ... ) {
            delete p_t;
            p_t = NULL;
            throw;
        }
    }

    ///
    /// Destructor
    ///
    ~RepetitiveRunnable() {
        stop();
        if(p_t) {
            p_t->join();
        }
        delete p_t;
    }

    ///
    /// \brief Call this to stop. If not called, it will be called by the destructor
    ///
    inline void stop() {
        if(!mustStop) {
            std::unique_lock<std::mutex> _(mutex);
            mustStop = true;
            cond.notify_one();
        }
    }

private:
    u_int32_t sleepTime;
    bool mustStop;
    std::mutex mutex;
    std::condition_variable cond;
    std::function<void()> f;
    std::thread * p_t;
};

} }

#endif

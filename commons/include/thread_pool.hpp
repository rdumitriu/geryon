///
/// \file thread_pool.hpp
///
/// Created on: Feb 21, 2014
/// Author: rdumitriu at gmail.com
///

#ifndef GERYON_THREAD_POOL_HPP_
#define GERYON_THREAD_POOL_HPP_

#include <vector>
#include <memory>
#include <future>

#include "monitor_queue.hpp"

#include "log.hpp"

namespace geryon { namespace mt {

namespace detail {
    template <typename T> class QTPMessage;
    enum Action { QUIT, PROCESS };
}

///
/// \brief Fixed pool of threads
///
/// A fixed pool of threads, each worker thread takes a job from the queue and executes it.\n
/// The type T must be copyable and represents the data the function is processing (all tasks will do the same).
///
template <typename T>
class QueuedThreadPool {
public:

    ///Constructor.
    /// \param f the function to be run (e.g. \code void f(const T& _t)) \endcode )
    /// \param _nT the number of threads
    /// \param _nQ the queue size, usually at least as the no of threads
    explicit
    QueuedThreadPool(const std::function<void(const T&)> & f, std::size_t _nT = 2, std::size_t _nQ = 2) : queue(_nQ) {
        for(std::size_t i = 0; i < _nT; ++i) {
            std::shared_ptr<std::thread> pt(new std::thread([this, f](){
                bool mustRun = true;
                while(mustRun) {
                    try {
                        detail::QTPMessage<T> msg = queue.get();
                        switch(msg.action) {
                            case detail::Action::PROCESS:
                                f(msg.taskData);
                                break;
                            case detail::Action::QUIT:
                                mustRun = false;
                                break;
                        }
                    } catch(...) {
                        LOG(geryon::util::Log::ERROR) << "Error in queue thread pool. Skipping over.";
                    }
                }
            }));
            threads.push_back(pt);
        }
    }

    /// Non-copyable
    QueuedThreadPool(const QueuedThreadPool & copy) = delete;
    /// Non-copyable
    QueuedThreadPool & operator = (const QueuedThreadPool & other) = delete;

    ///Destructor
    ~QueuedThreadPool() {
        for(std::size_t i = 0; i < threads.size(); ++i) { //fill in the quit messages
            queue.put(detail::QTPMessage<T>());
        }
        for(auto tsp : threads) {
            tsp->join();
        }
    }

    ///Puts something in our buffer. Blocks if not enough space
    void execute(const T & _t) {
        queue.put(detail::QTPMessage<T>(detail::PROCESS, _t));
    }


private:
    MonitorQueue<detail::QTPMessage<T>> queue;
    std::vector<std::shared_ptr<std::thread>> threads;
};

namespace detail {

    ///
    /// The actual struct, wrapping the message and the action
    ///
    template <typename T>
    struct QTPMessage {
        int action;
        T taskData;

        QTPMessage(Action _a, const T & _td) : action(_a), taskData(_td) {}
        QTPMessage() : action(Action::QUIT) {}
    };
}

} } /* namespace */
#endif

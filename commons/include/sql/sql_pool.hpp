///
/// \file sql_pool.hpp
///
/// Contains the SQLPool related classes.
///
#ifndef SQLPOOL_HPP__
#define SQLPOOL_HPP__

#include <string>
#include <queue>
#include <memory>

#include "repetitive_runnable.hpp"

#include "log.hpp"

namespace geryon { namespace sql {

///
/// \brief Is something wrong with the pool ?
///
/// Exception thrown when something is wrong within our SQL. It will serve as a
/// base class for our SQL exception hierarchy
///
class SQLException : public std::runtime_error {
public:
    ///Constructor
    explicit SQLException(std::string msg) : std::runtime_error(msg) {}
    ///Destructor
    virtual ~SQLException() throw() {}
};

template <typename T>
class SQLPool;

template <typename T>
class SQLConnection;


namespace configuration {
///
/// \brief The connection maintainer, provides open, test, close operations to the pool.
///
/// Providers should implement this (PostgreSQL, MySQL, etc)
///
template <typename T>
class SQLConnectionOpsImpl {
public:
    ///
    /// The SQL maintainer constructor
    /// \param connectString the connect string
    ///
    explicit
    SQLConnectionOpsImpl(const std::string & _connectString)
                                            : connectString(_connectString) {
    }

    SQLConnectionOpsImpl(const SQLConnectionOpsImpl & other) = delete;
    SQLConnectionOpsImpl & operator = (const SQLConnectionOpsImpl & other) = delete;

    ///
    /// \brief Destructor
    //
    virtual ~SQLConnectionOpsImpl() {
    }

    ///
    /// Open a connection
    ///
    virtual std::shared_ptr<T> open() throw(SQLException) = 0;

    ///
    /// Close a connection
    ///
    virtual void close(std::shared_ptr<T> & conn) throw(SQLException) = 0;

    ///
    /// Test the connection
    ///
    virtual bool test(std::shared_ptr<T> & conn) throw(SQLException) = 0;

    ///
    /// Gets the connect string
    /// \return the connect string
    ///
    const std::string getConnectString() const {
        return connectString;
    }
private:
    std::string connectString;
};

}


namespace detail {
///
/// \brief The connection structure.
///
/// The actual structure kept by the pool. It is non-copyable.
/// This is not a simple connection, it's augmented with createdAt timestamp
///
template <typename T>
class SQLConnectionInternal {
private:
    std::shared_ptr<T> connection;
    std::chrono::system_clock::time_point createdTimeStamp;
public:

    explicit SQLConnectionInternal() : connection(std::shared_ptr<T>(0)),
                                       createdTimeStamp(std::chrono::system_clock::now()) {}

    /**
     * \brief The SQL Connection constructor.
     *
     * We immediately compute the createdTS.
     * \param _connection the native connection pointer
     */
    explicit SQLConnectionInternal(std::shared_ptr<T> & _connection) :
                                             connection(_connection),
                                             createdTimeStamp(std::chrono::system_clock::now()) {
    }

    SQLConnectionInternal(const SQLConnectionInternal & other) {
        connection = other.connection;
        createdTimeStamp = other.createdTimeStamp;
    }

    SQLConnectionInternal & operator = (const SQLConnectionInternal & other) {
        if(this != & other) {
            this->connection = other.connection;
            this->createdTimeStamp = other.createdTimeStamp;
        }
        return *this;
    }

    /**
     * \brief Destructor.
     */
    virtual ~SQLConnectionInternal() {
    }

    /**
     * \brief The created timestamp
     * \return the created time stamp
     */
    const std::chrono::system_clock::time_point timeStamp() const {
        return createdTimeStamp;
    }

    /**
     * \brief Gets the reference to the connection
     * \return the connection reference
     */
    T & getConnection() {
        if(!connection.get()) {
            throw SQLException("Uninitialized connection!?!");
        }
        return *(connection);
    }

    /**
     * \brief Gets the reference to the connection
     * \return the connection reference
     */
    std::shared_ptr<T> connection_ptr() {
        return connection;
    }
};


template <typename T>
class SQLPoolMaintainer {
public:
    SQLPoolMaintainer(SQLPool<T> * _pPool) : pPool(_pPool) {}
    ~SQLPoolMaintainer() {}

    void operator()() {
        if(pPool->connTTL == 0) {
            return;
        }
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        LOG(geryon::util::Log::DEBUG) << "Pool cleaner running, queue size=" << pPool->queue.size();
        try {
            std::unique_lock<std::mutex> lock(pPool->mutex);
            //a. clean up old conns
            std::deque<detail::SQLConnectionInternal<T>> newq;
            for(typename std::deque<detail::SQLConnectionInternal<T>>::iterator i = pPool->queue.begin(); i != pPool->queue.end(); ++i) {
                detail::SQLConnectionInternal<T> & el = *i;
                std::chrono::seconds d(std::chrono::duration_cast<std::chrono::seconds>(now - el.timeStamp()));
                if(d.count() > pPool->connTTL) {
                    pPool->closeConnection(el); //no throws
                } else {
                    newq.push_back(el);
                }
            }
            pPool->queue = newq;
            //b. ensure minSz
            while(pPool->actualSize() < pPool->minSz) {
                std::shared_ptr<T> ptr_conn = pPool->rOpsImpl.open();
                detail::SQLConnectionInternal<T> conn(ptr_conn);
                pPool->queue.push_back(std::move(conn));
            }
        } catch( ... ) {
            LOG(geryon::util::Log::ERROR) << "Could not cleanup connections; will retry.";
        }
    }
private:
    SQLPool<T> * pPool;
};

}

///
/// \brief The SQL pool.
///
/// For now, we do not care about leaks detection (low priority, since we're working on the wrappers). Owned
/// connections not returned into the pool will represent, for sure, programming mistakes.
///
/// The SQLConnectionOpsImpl represents the implementation (the GoF bridge pattern) and it will be owned by the SQLPool
///
/// Access to the pool will only be made via SQLConnection
template <typename T>
class SQLPool {
public:

    ///Constructor.
    explicit
    SQLPool(unsigned int _minSz, unsigned int _maxSz,
            unsigned int _connTTL, unsigned int _checkInterval,
            bool _testOnBorrow, bool _testOnReturn,
            configuration::SQLConnectionOpsImpl<T> & _rOpsImpl)
                : minSz(_minSz), maxSz(_maxSz), connTTL(_connTTL),
                  testOnBorrow(_testOnBorrow), testOnReturn(_testOnReturn), noBorrowed(0),
                  rOpsImpl(_rOpsImpl), maintainer(_checkInterval, detail::SQLPoolMaintainer<T>(this)) {
        for(unsigned int i = 0; i < minSz; ++i) {
            std::shared_ptr<T> _bc = rOpsImpl.open();
            queue.push_back(std::move(detail::SQLConnectionInternal<T>(_bc)));
        }
        maintainer.start();
    }

    ///Destructor
    ~SQLPool() {
        maintainer.stop();
    }

    ///Gets the max size
    unsigned int maxSize() {
        return maxSz;
    }

    ///Gets the min size
    unsigned int minSize() {
        return minSz;
    }

    inline unsigned int usedSize() {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size() + noBorrowed;
    }

    /// Non-copyable
    SQLPool(const SQLPool & copy) = delete;
    /// Non-copyable
    SQLPool & operator = (const SQLPool & other) = delete;

    friend class SQLConnection<T>;
    friend class detail::SQLPoolMaintainer<T>;
protected:

    ///
    /// \brief acquire
    /// \return
    ///
    detail::SQLConnectionInternal<T> acquire() {
        std::unique_lock<std::mutex> lock(mutex);

        bool mustTry = true;
        detail::SQLConnectionInternal<T> conn;
        while(mustTry) {
            //do we have something ready ?
            if(!queue.empty()) {
                //yeap, just take the front element
                conn = queue.front();
                queue.pop_front();
            } else if(actualSize() < maxSz){
                //we must allocate a new one
                std::shared_ptr<T> connptr = rOpsImpl.open();
                conn = detail::SQLConnectionInternal<T>(connptr);
            } else {
                while(actualSize() == maxSz) { //depleted, we must wait
                    cond.wait(lock);
                }
            }
            if(testOnBorrow) {
                try {
                    std::shared_ptr<T> connptr = conn.connection_ptr();
                    if(rOpsImpl.test(connptr)) {
                        closeConnection(conn);
                        mustTry = false;
                    }
                } catch( ... ) {
                    closeConnection(conn);
                }
            } else {
                mustTry = false;
            }
        }
        noBorrowed++;
        //cond.notify_one(); - not needed, we do not wait on release
        return conn;
    }

    ///
    /// \brief release
    /// \param conn
    ///
    void release(detail::SQLConnectionInternal<T> & conn) {
        //first, we must check it, if needed
        std::unique_lock<std::mutex> lock(mutex);

        if(testOnReturn) {
            try {
                std::shared_ptr<T> connptr = conn.connection_ptr();
                if(rOpsImpl.test(connptr)) {
                    queue.push_back(conn);
                } else {
                    closeConnection(conn);
                }
            } catch( ... ) {
                closeConnection(conn);
            }
        } else {
            queue.push_back(conn);
        }

        noBorrowed--;
        cond.notify_one();
    }

    //no-lock
    inline unsigned int actualSize() {
        return queue.size() + noBorrowed;
    }

    void closeConnection(detail::SQLConnectionInternal<T> & conn) {
        try {
            std::shared_ptr<T> connptr = conn.connection_ptr();
            rOpsImpl.close(connptr);
        } catch( ... ) {} //ignore
    }

private:

    std::condition_variable cond; /*condition */
    std::mutex mutex; /* the mutex */
    unsigned int minSz;
    unsigned int maxSz;
    unsigned int connTTL;
    bool testOnBorrow;
    bool testOnReturn;
    std::deque<detail::SQLConnectionInternal<T>> queue;
    unsigned int noBorrowed; /* we do not care for leaks, so this is enough*/

    configuration::SQLConnectionOpsImpl<T> & rOpsImpl; /* implementation */
    geryon::mt::RepetitiveRunnable maintainer;
};

///
/// The wrapper over the basic connection. Movable, but not copyable
///
template <typename T>
class SQLConnection {
public:
    SQLConnection() : pool(0) {
    }

    SQLConnection(SQLPool<T> & _pool) : pool(&_pool), conn(pool->acquire()) {
    }

    ~SQLConnection() {
        if(pool) {
            pool->release(conn);
        }
    }

    /// Movable
    SQLConnection(const SQLConnection && copy) {
        pool = copy.pool;
        conn = copy.conn;
        copy.pool = 0;
    }

    /// Movable
    SQLConnection & operator =(const SQLConnection && copy) {
        if(this != &copy) {
            if(pool) {
                //we must release current
                pool->release(conn);
            }
            pool = copy.pool;
            conn = copy.conn;
            copy.pool = 0;
        }
        return (*this);
    }

    /// Non-copyable
    SQLConnection(const SQLConnection & copy) = delete;
    /// Non-copyable
    SQLConnection & operator = (const SQLConnection & other) = delete;

    T & connection() {
        if(!pool) {
            throw SQLException("Usage of un-initialized connection!");
        }
        return conn.getConnection();
    }

private:
    SQLPool<T> * pool;
    detail::SQLConnectionInternal<T> conn;
};

} }

#endif

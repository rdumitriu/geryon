
#include "sql/sql_pool.hpp"

class SQLConnectionOpsImpl_T : public geryon::sql::configuration::SQLConnectionOpsImpl<int> {

public:
    SQLConnectionOpsImpl_T(const std::string & connstr) : geryon::sql::configuration::SQLConnectionOpsImpl<int>(connstr), i(0) {}
    virtual ~SQLConnectionOpsImpl_T() {
    }

    ///
    /// Open a connection
    ///
    virtual std::shared_ptr<int> open() throw(geryon::sql::SQLException) {
        std::shared_ptr<int> ret = std::make_shared<int>(i);
        LOG(geryon::util::Log::INFO) << "CREATE::" << *ret;
        i++;
        return ret;
    }


    ///
    /// Close a connection
    ///
    virtual void close(std::shared_ptr<int> & conn) throw(geryon::sql::SQLException) {
        LOG(geryon::util::Log::INFO) << "CLOSE::" << *conn;
    }

    ///
    /// Test the connection
    ///
    virtual bool test(std::shared_ptr<int> & conn) throw(geryon::sql::SQLException) {
        return true;
    }

private:
    int i;
};

typedef geryon::sql::SQLPool<int> SQLPool_T;


int main(int argn, const char * argv []) {
    SQLConnectionOpsImpl_T impl("bla");
    SQLPool_T pool(1, 3, 3, 5, false, false, impl);

    geryon::sql::SQLConnection<int> c1(pool);
    geryon::sql::SQLConnection<int> c2(pool);
    int x = -1;
    {
        LOG(geryon::util::Log::INFO) << "Getting the 3rd conn!";
        geryon::sql::SQLConnection<int> c3(pool);
        x = c3.connection();
    }
    LOG(geryon::util::Log::INFO) << "Getting the 3rd conn (2)!";
    geryon::sql::SQLConnection<int> c3(pool);
    LOG(geryon::util::Log::INFO) << "Got it, finishing it up!";
    if(x != c3.connection()) {
        LOG(geryon::util::Log::ERROR) << "FAILED : should reuse it!";
    }
    LOG(geryon::util::Log::INFO) << "End";

    return 0;
}

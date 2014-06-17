
#ifdef G_HAS_PQXX

#include "postgres_support.hpp"

namespace geryon { namespace server { namespace detail {


std::shared_ptr<pqxx::connection> PostgresSQLConnectionOpsImpl::open() throw(geryon::sql::SQLException) {
    try {
        LOG(geryon::util::Log::DEBUG) << "Connect using the connect string:" << getConnectString();
        return std::make_shared<pqxx::connection>(getConnectString());
    } catch(std::runtime_error & e) {
        std::ostringstream msgs;
        msgs << "Getting connection failed, error was :" << e.what() <<". Will throw away.";
        std::string msg = msgs.str();
        LOG(geryon::util::Log::ERROR) << msg;
        throw geryon::sql::SQLException(msg);
    }
}

void PostgresSQLConnectionOpsImpl::close(std::shared_ptr<pqxx::connection> & conn) throw(geryon::sql::SQLException) {
    try {
        conn->disconnect();
    } catch(std::runtime_error & e) {
        std::ostringstream msgs;
        msgs << "Closing connection failed, error was :" << e.what() <<".";
        std::string msg = msgs.str();
        LOG(geryon::util::Log::ERROR) << msg;
    }
}

bool PostgresSQLConnectionOpsImpl::test(std::shared_ptr<pqxx::connection> & conn) throw(geryon::sql::SQLException) {
    try {
        if(!conn.get() || !conn->is_open()) {
            return false;
        }
        pqxx::nontransaction uow(*conn);
        pqxx::result r = uow.exec("SELECT 1"); //if this doesn't fail we are doing great!
        return (r.size() == 1); //let's suppose we do not need to check the value of 1
    } catch(std::runtime_error & e) {
        std::ostringstream msgs;
        msgs << "Testing connection failed, error was :" << e.what() <<".";
        std::string msg = msgs.str();
        LOG(geryon::util::Log::ERROR) << msg;
    }
    return false;
}

} } }

#endif

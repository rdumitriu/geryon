///
/// \file postgres_support.hpp
///
///  Created on: Jun 05, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYONCONFIGSQL_POSTGRES_HPP_
#define GERYONCONFIGSQL_POSTGRES_HPP_

#include <memory>

#ifdef G_HAS_PQXX

#include "sql/sql_postgres_support.hpp"

namespace geryon { namespace server {

namespace detail {


class PostgresSQLConnectionOpsImpl : public geryon::sql::configuration::SQLConnectionOpsImpl<pqxx::connection> {
public:
    PostgresSQLConnectionOpsImpl(const std::string & connectString)
                        : geryon::sql::configuration::SQLConnectionOpsImpl<pqxx::connection>(connectString) {}
    virtual ~PostgresSQLConnectionOpsImpl() {}

    virtual std::shared_ptr<pqxx::connection> open() throw(geryon::sql::SQLException);

    virtual void close(std::shared_ptr<pqxx::connection> & conn) throw(geryon::sql::SQLException);

    virtual bool test(std::shared_ptr<pqxx::connection> & conn) throw(geryon::sql::SQLException);
};

}

class PostgresInternalPoolImpl {
public:
    PostgresInternalPoolImpl(const std::string & connectString,
                             unsigned int minSz, unsigned int maxSz,
                             unsigned int connTTL, unsigned int checkInterval,
                             bool testOnBorrow, bool testOnReturn)
                                  : ops_impl(connectString),
                                    pool(std::make_shared<geryon::sql::postgres::PostgresConnectionPool>(minSz, maxSz,
                                                                                                         connTTL,
                                                                                                         checkInterval,
                                                                                                         testOnBorrow,
                                                                                                         testOnReturn,
                                                                                                         ops_impl)) {}
    ~PostgresInternalPoolImpl() {}
private:
    detail::PostgresSQLConnectionOpsImpl ops_impl;
public:
    std::shared_ptr<geryon::sql::postgres::PostgresConnectionPool> pool;
};

} }

#endif

#endif

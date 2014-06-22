///
/// \file postgres_support.hpp
///
///  Created on: Jun 05, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYON_CONFIGSQL_POSTGRES_HPP_
#define GERYON_CONFIGSQL_POSTGRES_HPP_

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

} }

#endif

#endif

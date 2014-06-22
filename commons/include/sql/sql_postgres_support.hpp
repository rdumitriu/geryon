/**
 * \file sql_postgres_support.hpp
 * 
 * Contains the tailoring of the SQLPool related classes to Postgres.
 * 
 * Created on: Jun 04, 2014
 * Author: rdumitriu at gmail.com
 */
#ifndef GERYON_SQL_POSTGRES_SUPPORT_HPP_
#define GERYON_SQL_POSTGRES_SUPPORT_HPP_

#include "sql/sql_pool.hpp"

#include <pqxx/pqxx>

namespace geryon { namespace sql { namespace postgres {
    
/* ==========================================================================
 *  POSTGRESQL SUPPORT
 * ========================================================================== */
///
/// \brief The connection
///
typedef SQLConnection<pqxx::connection> PostgresConnection;

///
/// \brief The pool
///
typedef SQLPool<pqxx::connection> PostgresConnectionPool;
typedef std::shared_ptr<geryon::sql::postgres::PostgresConnectionPool> PostgresConnectionPoolPtr;

} } } /* namespace */

#endif

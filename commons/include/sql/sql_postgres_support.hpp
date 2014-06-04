/**
 * \file sql_postgres_support.hpp
 * 
 * Contains the tailoring of the SQLPool related classes to Postgres.
 * 
 * Created on: Jun 04, 2014
 * Author: rdumitriu at gmail.com
 */
#ifndef SQL_POSTGRES_SUPPORT_HPP_
#define SQL_POSTGRES_SUPPORT_HPP_

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


} } } /* namespace */

#endif

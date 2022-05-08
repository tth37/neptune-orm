#include "neptune/connection.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

neptune::connection::connection() : m_should_close(false) {}

neptune::connection::query_selector neptune::connection::query() { return {}; }

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : m_sql_conn(std::move(conn)) {}

neptune::mariadb_connection::~mariadb_connection() {
  m_should_close = true;
  std::unique_lock<std::mutex> lock(m_conn_mtx);
  m_sql_conn->close();
}

void neptune::mariadb_connection::insert(const entity &entity) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_conn_mtx);
  try {
    m_sql_conn->createStatement()->execute(entity.get_insert_sql());
    __NEPTUNE_LOG(debug, "Inserted entity: {}" + entity.get_insert_sql());
  } catch (sql::SQLException &e) {
    __NEPTUNE_LOG(warn, "Table structure may be outdated");
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}
std::vector<neptune::entity>
neptune::mariadb_connection::run_sql(neptune::entity &e,
                                     const std::string &sql) {

  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }

  std::unique_lock<std::mutex> lock(m_conn_mtx);

  try {
    auto stmt = m_sql_conn->createStatement();
    stmt->execute(sql + ";");
    auto res = stmt->getResultSet();
    std::vector<neptune::entity> entities;
    while (res->next()) {
      for (const auto &col : e.get_cols()) {
        __NEPTUNE_LOG(debug, col->get_col_name());
        std::string value = (std::string)res->getString(col->get_col_name());
        col->set_value_from_string(value);
        __NEPTUNE_LOG(debug, "Set value: " + value);
      }
      entities.push_back(e);
    }
    return entities;
  } catch (sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

// std::vector<neptune::entity>
// neptune::mariadb_connection::run_sql(const std::string &sql) {
//   if (m_should_close) {
//     __NEPTUNE_THROW(exception_type::runtime_error, "Connection already
//     closed");
//   }
//   std::unique_lock<std::mutex> lock(m_conn_mtx);
//   try {
//     auto stmt = m_sql_conn->createStatement();
//     stmt->execute(sql);
//     auto res = stmt->getResultSet();
//     std::vector<entity> entities;
//     while (res->next()) {
//       entity e;
//     }
//     return entities;
//   } catch (sql::SQLException &e) {
//     __NEPTUNE_THROW(exception_type::sql_error, e.what());
//   }
// }

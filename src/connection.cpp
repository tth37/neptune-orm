#include "neptune/connection.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

// =============================================================================
// neptune::connection =========================================================
// =============================================================================

neptune::connection::connection() : m_should_close(false) {}

neptune::connection::query_selector neptune::connection::query() { return {}; }

// =============================================================================
// neptune::mariadb_connection =================================================
// =============================================================================

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : connection(), m_conn(std::move(conn)) {}

neptune::mariadb_connection::~mariadb_connection() {
  if (m_should_close) {
    __NEPTUNE_LOG(warn, "Connection already closed");
  }
  m_should_close = true;
  std::unique_lock<std::mutex> lock(m_mtx);
  m_conn->close();
}

void neptune::mariadb_connection::insert(const neptune::entity &e) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = e.get_insert_sql_mariadb();
  __NEPTUNE_LOG(debug, "Insert SQL: " + sql);
  try {
    m_conn->createStatement()->execute(sql);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::mariadb_connection::select_entities(
    neptune::entity &e, const neptune::connection::query_selector &selector) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = parse_selector(e, selector);
  __NEPTUNE_LOG(debug, "Select SQL: " + sql);
  std::vector<std::shared_ptr<neptune::entity>> res;
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
    auto res_set = stmt->getResultSet();
    while (res_set->next()) {
      auto cur_e = e.duplicate();
      for (auto col : cur_e->get_cols()) {
        std::string value =
            (std::string)res_set->getString(col->get_col_name());
        col->set_value_from_sql_mariadb(value);
      }
      res.push_back(cur_e);
    }
    return res;
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}
std::string neptune::mariadb_connection::parse_selector(
    const neptune::entity &e,
    const neptune::connection::query_selector &selector) {
  std::string res = "SELECT * FROM `" + e.get_table_name() + "`";
  res += ";";
  return res;
}

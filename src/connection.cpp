#include "neptune/connection.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

// =============================================================================
// neptune::connection =========================================================
// =============================================================================

neptune::connection::connection() : m_mutex(), m_should_close(false) {}

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : connection(), m_conn(std::move(conn)) {}

neptune::mariadb_connection::~mariadb_connection() {
  if (m_should_close) {
    __NEPTUNE_LOG(warn, "Connection already closed");
  }
  m_should_close = true;
  std::unique_lock<std::mutex> lock(m_mutex);
  m_conn->close();
}

void neptune::mariadb_connection::execute(const std::string &sql) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mutex);
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::mariadb_connection::execute(
    const std::string &sql,
    std::function<std::shared_ptr<entity>()> duplicate) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mutex);
  try {
    auto stmt = m_conn->createStatement();
    auto res = stmt->executeQuery(sql);
    std::vector<std::shared_ptr<neptune::entity>> ret;
    while (res->next()) {
      auto e = duplicate();
      for (const auto &col_meta : e->get_col_metas()) {
        std::string value = (std::string)res->getString(col_meta.name);
        if (!value.empty())
          e->set_col_data_from_string(col_meta.name, value);
      }
    }
    return ret;
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

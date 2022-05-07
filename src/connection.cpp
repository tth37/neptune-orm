#include "neptune/connection.hpp"

#include <utility>

neptune::connection::connection() : m_in_use(false), m_should_close(false) {}

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : m_sql_conn(std::move(conn)) {}

neptune::mariadb_connection::~mariadb_connection() {
  m_should_close = true;
  std::unique_lock<std::mutex> lock(m_conn_mtx);
  m_conn_cv.wait(lock, [this] { return !m_in_use; });
  m_sql_conn->close();
}

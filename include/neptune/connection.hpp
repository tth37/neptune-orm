#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include <mariadb/conncpp/Connection.hpp>
#include <mutex>

namespace neptune {
class connection {
public:
  connection();

  virtual ~connection() = default;

protected:
  std::mutex m_conn_mtx;
  std::atomic<bool> m_should_close, m_in_use;
  std::condition_variable m_conn_cv;
};

class mariadb_connection : public connection {
public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);

  ~mariadb_connection() override;

private:
  std::shared_ptr<sql::Connection> m_sql_conn;
};
} // namespace neptune

#endif // NEPTUNEORM_CONNECTION_HPP

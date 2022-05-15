#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include <functional>
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>
#include <set>

namespace neptune {

class connection {
public:
  connection();

  virtual ~connection() = default;

  template <typename T> void insert(T &e);

protected:
  std::mutex m_mutex;
  std::atomic<bool> m_should_close;

private:
  virtual void execute(const std::string &sql) = 0;

  virtual std::vector<std::shared_ptr<entity>>
  execute(const std::string &sql,
          std::function<std::shared_ptr<entity>()> duplicate) = 0;
};

class mariadb_connection : public connection {
public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);

  ~mariadb_connection() override;

private:
  std::shared_ptr<sql::Connection> m_conn;

  void execute(const std::string &sql) override;

  std::vector<std::shared_ptr<entity>>
  execute(const std::string &sql,
          std::function<std::shared_ptr<entity>()> duplicate) override;
};

} // namespace neptune

template <typename T> void neptune::connection::insert(T &e) {}

#endif // NEPTUNEORM_CONNECTION_HPP

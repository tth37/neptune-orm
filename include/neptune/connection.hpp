#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/query_selector.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/parser.hpp"
#include "neptune/utils/uuid.hpp"
#include <functional>
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>
#include <set>

namespace neptune {

class connection {
  /**
   * class connection
   * An abstract class to interact with database.
   *
   * Virtual function "exec" is used to execute SQL statements, and virtual
   * function "fetch" is used to fetch data from database.
   */
private:
  virtual void exec(const std::string &sql) = 0;
  virtual std::vector<std::shared_ptr<entity>>
  fetch(const std::string &sql,
        std::function<std::shared_ptr<entity>()> duplicate,
        const std::set<std::string> &select_set) = 0;

public:
  connection() = default;
  virtual ~connection() = default;
  template <typename T> std::shared_ptr<T> insert(const std::shared_ptr<T> &e);
  template <typename T>
  std::vector<std::shared_ptr<T>> select(const query_selector &selector);
  template <typename T> void update(const std::shared_ptr<T> &e);
  template <typename T> void remove(const std::shared_ptr<T> &e);
};

class mariadb_connection : public connection {
private:
  std::shared_ptr<sql::Connection> m_conn;
  void exec(const std::string &sql) override;
  std::vector<std::shared_ptr<entity>>
  fetch(const std::string &sql,
        std::function<std::shared_ptr<entity>()> duplicate,
        const std::set<std::string> &select_set) override;

public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);
  ~mariadb_connection() override = default;
};

} // namespace neptune

// =============================================================================
// neptune::connection =========================================================
// =============================================================================

template <typename T>
std::shared_ptr<T> neptune::connection::insert(const std::shared_ptr<T> &e) {
  auto uuid = uuid::uuid();
  e->uuid.set_value(uuid);
  std::string sql = parser::insert_entity(e);
  exec(sql);
  sql = parser::query_last_insert_entity(e, uuid);
  auto inserted_e = fetch(
      sql, []() { return std::make_shared<T>(); },
      parser::get_default_select_set(e));
  if (inserted_e.size() != 1) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Insert failed");
  }
  return std::dynamic_pointer_cast<T>(inserted_e[0]);
}

template <typename T>
std::vector<std::shared_ptr<T>>
neptune::connection::select(const neptune::query_selector &selector) {
  auto e = std::make_shared<T>();
  auto select_set = parser::get_select_set(e, selector);
  std::string sql = parser::select_entities(e, selector);
}

// template <typename T>
// std::vector<std::shared_ptr<T>>
// neptune::connection::select(const neptune::query_selector &selector) {
//   auto e = std::make_shared<T>();
// }

#endif // NEPTUNEORM_CONNECTION_HPP

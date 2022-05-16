#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/query_selector.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/utils.hpp"
#include <functional>
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>
#include <set>

namespace neptune {

class connection {
public:
  connection();

  virtual ~connection() = default;

  template <typename T> std::shared_ptr<T> insert(const std::shared_ptr<T> &e);

  template <typename T>
  std::vector<std::shared_ptr<T>> select(query_selector &selector);

  template <typename T> std::shared_ptr<T> select_one(query_selector &selector);

protected:
  std::mutex m_mutex;
  std::atomic<bool> m_should_close;

private:
  virtual void execute(const std::string &sql) = 0;

  virtual std::vector<std::shared_ptr<entity>>
  execute(const std::string &sql,
          std::function<std::shared_ptr<entity>()> duplicate,
          const std::set<std::string> &select_cols = {}) = 0;

  static std::string parse_insert_entity_sql(const std::shared_ptr<entity> &e);

  static std::string
  parse_query_last_insert_entity_sql(const std::shared_ptr<entity> &e,
                                     const std::string &uuid);

  static std::string parse_select_entities_sql(const std::shared_ptr<entity> &e,
                                               const query_selector &selector);
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
          std::function<std::shared_ptr<entity>()> duplicate,
          const std::set<std::string> &select_cols) override;
};

} // namespace neptune

template <typename T>
std::shared_ptr<T> neptune::connection::insert(const std::shared_ptr<T> &e) {
  auto uuid = neptune::utils::uuid();
  e->uuid.set_value(uuid);
  std::string sql = parse_insert_entity_sql(e);
  execute(sql);
  sql = parse_query_last_insert_entity_sql(e, uuid);
  auto inserted_e = execute(sql, []() { return std::make_shared<T>(); });
  if (inserted_e.size() != 1) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Insert failed");
  }
  return std::static_pointer_cast<T>(inserted_e[0]);
}

template <typename T>
std::vector<std::shared_ptr<T>>
neptune::connection::select(query_selector &selector) {
  auto e = std::make_shared<T>();
  auto col_metas = e->get_col_metas();
  auto select_cols = selector.get_select_cols_set();

  std::string sql = parse_select_entities_sql(e, selector);
  auto raw_res = execute(
      sql, []() { return std::make_shared<T>(); }, select_cols);

  std::vector<std::shared_ptr<T>> res;
  for (auto &r : raw_res) {
    res.push_back(std::static_pointer_cast<T>(r));
  }
  return res;
}

template <typename T>
std::shared_ptr<T> neptune::connection::select_one(query_selector &selector) {
  auto res = select<T>(selector.limit(1));
  if (res.empty())
    return nullptr;
  return res[0];
}

#endif // NEPTUNEORM_CONNECTION_HPP

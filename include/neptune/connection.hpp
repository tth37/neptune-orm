#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>

namespace neptune {

class connection {
public:
  connection();

  virtual ~connection() = default;

  virtual void insert(const neptune::entity &e) = 0;

protected:
  std::mutex m_mtx;
  std::atomic<bool> m_should_close;

  class query_selector {};

public:
  /**
   * Generate an empty selector
   * @return query_selector
   */
  static query_selector query();

  template <typename T> std::vector<T> select(const query_selector &selector) {
    T e;
    auto raw_res = select_entities(e, selector);
    std::vector<T> res;
    for (auto &r : raw_res) {
      res.push_back(*std::static_pointer_cast<T>(r));
    }
    return res;
  }

private:
  virtual std::vector<std::shared_ptr<entity>>
  select_entities(neptune::entity &e, const query_selector &selector) = 0;
};

class mariadb_connection : public connection {
public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);

  ~mariadb_connection() override;

  void insert(const neptune::entity &e) override;

private:
  std::shared_ptr<sql::Connection> m_conn;

  std::vector<std::shared_ptr<neptune::entity>>
  select_entities(neptune::entity &e, const query_selector &selector) override;

  static std::string parse_selector(const neptune::entity &e,
                                    const query_selector &selector);
};

} // namespace neptune

#endif // NEPTUNEORM_CONNECTION_HPP

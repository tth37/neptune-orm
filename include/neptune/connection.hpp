#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>

namespace neptune {
class connection {
protected:
  class query_selector {
    friend class connection;

  public:
    query_selector() = default;

    std::string get_sql() const {
      std::string sql;
      if (!m_where_sql.empty()) {
        sql += " WHERE " + m_where_sql;
      }
      if (!m_order_by_sql.empty()) {
        sql += " ORDER BY " + m_order_by_sql;
      }
      if (!m_limit_sql.empty()) {
        sql += " LIMIT " + m_limit_sql;
      }
      if (!m_offset_sql.empty()) {
        sql += " OFFSET " + m_offset_sql;
      }
      return sql;
    }

    query_selector &where(const std::string &col, const std::string &op,
                          const std::string &val) {
      if (!m_where_sql.empty())
        m_where_sql += " AND ";
      m_where_sql += col + " " + op + " \"" + val + "\"";
      return *this;
    }

    query_selector &where(const std::string &col, const std::string &op,
                          const int val) {
      if (!m_where_sql.empty())
        m_where_sql += " AND ";
      m_where_sql += col + " " + op + " " + std::to_string(val);
      return *this;
    }

    query_selector &where(const std::string &col, const std::string &op,
                          const double val) {
      if (!m_where_sql.empty())
        m_where_sql += " AND ";
      m_where_sql += col + " " + op + " " + std::to_string(val);
      return *this;
    }

    query_selector &order_by(const std::string &col, const std::string &order) {
      if (!(order == "ASC" || order == "DESC")) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Order must be ASC or DESC");
      }
      if (!m_order_by_sql.empty())
        m_order_by_sql += ", ";
      m_order_by_sql += col + " " + order;
      return *this;
    }

    query_selector &limit(const int limit) {
      m_limit_sql = std::to_string(limit);
      return *this;
    }

    query_selector &offset(const int offset) {
      m_offset_sql = std::to_string(offset);
      return *this;
    }

  private:
    std::string m_where_sql, m_order_by_sql, m_limit_sql, m_offset_sql;
  };

public:
  connection();

  virtual ~connection() = default;

  virtual void insert(const entity &entity) = 0;

  static query_selector query();

  virtual std::vector<entity> run_sql(entity &e, const std::string &sql) = 0;

  template <typename T> std::vector<T> select(const query_selector &query) {
    T instance;
    std::string sql = "SELECT * FROM " + instance.get_table_name() + " ";
    sql += query.get_sql();
    auto entities = run_sql(instance, sql);
    std::vector<T> results;
    results.reserve(entities.size());
    for (auto &entity : entities) {
      results.push_back(entity.template get<T>());
    }
    return results;
  }

protected:
  std::mutex m_conn_mtx;
  std::atomic<bool> m_should_close;
};

class mariadb_connection : public connection {
public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);

  ~mariadb_connection() override;

  void insert(const entity &entity) override;

  std::vector<entity> run_sql(entity &e, const std::string &sql) override;

private:
  std::shared_ptr<sql::Connection> m_sql_conn;
};
} // namespace neptune

#endif // NEPTUNEORM_CONNECTION_HPP

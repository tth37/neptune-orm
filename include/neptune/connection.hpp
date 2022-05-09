#ifndef NEPTUNEORM_CONNECTION_HPP
#define NEPTUNEORM_CONNECTION_HPP

#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include <mariadb/conncpp/Connection.hpp>
#include <mutex>
#include <set>

namespace neptune {

class connection {
public:
  connection();

  virtual ~connection() = default;

  virtual void insert(const neptune::entity &e) = 0;

protected:
  std::mutex m_mtx;
  std::atomic<bool> m_should_close;

public:
  class query_selector {
    friend class connection;

  public:
    query_selector();

  public:
    struct _where_clause {
      _where_clause(std::string col_, std::string op_, std::string val_);

      _where_clause(std::string col_, std::string op_, int32_t val_);

      std::string col, op, val;
    };

  public:
    struct _where_clause_tree_node {
      std::string op;
      _where_clause clause;
      std::shared_ptr<_where_clause_tree_node> left, right;

      _where_clause_tree_node();
    };

  private:
    struct _order_by_clause {
      std::string col, dir;
    };

  public:
    query_selector &where(const _where_clause &where_clause);

    query_selector &where(const std::shared_ptr<_where_clause_tree_node> &where_clause_root);

    query_selector &order_by(_order_by_clause order_by_clause);

    query_selector &limit(std::size_t limit);

    query_selector &offset(std::size_t offset);

    query_selector &confirm_no_where();

  public:
    std::shared_ptr<_where_clause_tree_node> m_where_clause_root;
    std::vector<_order_by_clause> m_order_by_clauses;
    std::size_t m_limit, m_offset;
    bool m_has_limit, m_has_offset;
    bool m_confirm_no_where;
  };


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

  template <typename T> void remove(const query_selector &selector) {
    T e;
    remove_entities(e, selector);
  }

  virtual void update(const neptune::entity &e,
                      const query_selector &selector) = 0;

private:
  virtual std::vector<std::shared_ptr<entity>>
  select_entities(neptune::entity &e, const query_selector &selector) = 0;

  virtual void remove_entities(neptune::entity &e,
                               const query_selector &selector) = 0;
};

class mariadb_connection : public connection {
public:
  explicit mariadb_connection(std::shared_ptr<sql::Connection> conn);

  ~mariadb_connection() override;

  void insert(const neptune::entity &e) override;

  void update(const neptune::entity &e,
              const query_selector &selector) override;

private:
  std::shared_ptr<sql::Connection> m_conn;

  std::vector<std::shared_ptr<neptune::entity>>
  select_entities(neptune::entity &e, const query_selector &selector) override;

  void remove_entities(neptune::entity &e,
                       const query_selector &selector) override;

  static std::string parse_selector_query(const neptune::entity &e,
                                          const query_selector &selector);

  static std::string
  parse_selector_update_remove(const neptune::entity &e,
                               const query_selector &selector);

  static std::string _dfs_parse_selector_where_clause_tree(
      const std::set<std::string> &col_names,
      const std::shared_ptr<query_selector::_where_clause_tree_node> &node);
};

std::shared_ptr<connection::query_selector::_where_clause_tree_node>
or_(const std::shared_ptr<connection::query_selector::_where_clause_tree_node> &left,
    const std::shared_ptr<connection::query_selector::_where_clause_tree_node> &right);

std::shared_ptr<connection::query_selector::_where_clause_tree_node>
or_(const std::shared_ptr<connection::query_selector::_where_clause_tree_node> &left,
    const connection::query_selector::_where_clause &right_where_clause);

std::shared_ptr<connection::query_selector::_where_clause_tree_node>
or_(const connection::query_selector::_where_clause &left_where_clause,
    const std::shared_ptr<connection::query_selector::_where_clause_tree_node> &right);

std::shared_ptr<connection::query_selector::_where_clause_tree_node>
or_(const connection::query_selector::_where_clause &left_where_clause,
    const connection::query_selector::_where_clause &right_where_clause);

} // namespace neptune

#endif // NEPTUNEORM_CONNECTION_HPP

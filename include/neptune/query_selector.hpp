#ifndef NEPTUNEORM_QUERY_SELECTOR_HPP
#define NEPTUNEORM_QUERY_SELECTOR_HPP

#include "entity.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace neptune {

class query_selector {

private:
  struct _where_clause {
    _where_clause(std::string col_, std::string op_, std::string val_);

    _where_clause(std::string col_, std::string op_, int32_t val_);

    _where_clause();

    std::string col, op, val;
  };

  struct _where_clause_tree_node {
    _where_clause_tree_node();

    std::string op;
    _where_clause clause;
    std::shared_ptr<_where_clause_tree_node> left, right;
  };

  struct _order_by_clause {
    std::string col, dir;
  };

  std::shared_ptr<_where_clause_tree_node> m_where_clause_root;
  std::vector<_order_by_clause> m_order_by_clauses;
  std::size_t m_limit, m_offset;
  bool m_has_limit, m_has_offset;
  bool m_confirm_no_where;

  std::string _dfs_parse_where_clause_tree(
      const std::shared_ptr<_where_clause_tree_node> &node,
      const std::set<std::string> &col_names);

  std::string parse_where(const entity &e);

  std::string parse_query(const entity &e);

public:
  query_selector();

  query_selector &where(const _where_clause &where_clause);

  query_selector &where(const std::string &col, const std::string &op,
                        const std::string &val);

  query_selector &where(const std::string &col, const std::string &op,
                        int32_t val);

  query_selector &
  where(const std::shared_ptr<_where_clause_tree_node> &where_clause_root);

  query_selector &order_by(_order_by_clause order_by_clause);

  query_selector &limit(std::size_t limit);

  query_selector &offset(std::size_t offset);

  query_selector &confirm_no_where();

  static std::shared_ptr<query_selector::_where_clause_tree_node>
  or_(const std::shared_ptr<query_selector::_where_clause_tree_node> &left,
      const std::shared_ptr<query_selector::_where_clause_tree_node> &right);

  static std::shared_ptr<query_selector::_where_clause_tree_node>
  or_(const std::shared_ptr<query_selector::_where_clause_tree_node> &left,
      const query_selector::_where_clause &right_where_clause);

  static std::shared_ptr<query_selector::_where_clause_tree_node>
  or_(const query_selector::_where_clause &left_where_clause,
      const std::shared_ptr<query_selector::_where_clause_tree_node> &right);

  static std::shared_ptr<query_selector::_where_clause_tree_node>
  or_(const query_selector::_where_clause &left_where_clause,
      const query_selector::_where_clause &right_where_clause);
};

} // namespace neptune

#endif // NEPTUNEORM_QUERY_SELECTOR_HPP

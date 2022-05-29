#ifndef NEPTUNEORM_QUERY_SELECTOR_HPP
#define NEPTUNEORM_QUERY_SELECTOR_HPP

#include "entity.hpp"
#include "neptune/utils/typedefs.hpp"
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace neptune {

class query_selector {
  friend class connection;
  friend class parser;

  /**
   * class query_selector
   * A class to store query metadata.
   */

private:
  struct where_clause {
    where_clause(std::string col_, std::string op_, std::string val_);
    where_clause(std::string col_, std::string op_, std::int32_t val_);
    where_clause(std::string col_, std::string op_, std::uint32_t val_);
    where_clause();

    std::string col, op, val;
  };

private:
  struct where_clause_tree_node {
    where_clause_tree_node(std::string op_, where_clause clause,
                           std::shared_ptr<where_clause_tree_node> left_,
                           std::shared_ptr<where_clause_tree_node> right_);

    std::string op;
    where_clause clause;
    std::shared_ptr<where_clause_tree_node> left, right;
  };

private:
  struct order_by_clause {
    order_by_clause(std::string col_, order_dir dir_);

    std::string col;
    order_dir dir;
  };

private:
  std::shared_ptr<where_clause_tree_node> m_where_clause_root;
  std::vector<order_by_clause> m_order_by_clauses;
  std::set<std::string> m_select_cols, m_select_rels;
  std::size_t m_limit{}, m_offset{};
  bool m_has_limit, m_has_offset;

private:
  struct where_clause_tree_node_helper {
    where_clause_tree_node_helper(
        std::shared_ptr<where_clause_tree_node> node_);
    where_clause_tree_node_helper(const where_clause &clause_);
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::string val_);
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::int32_t val_);
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::uint32_t val_);
    [[nodiscard]] std::shared_ptr<where_clause_tree_node> get() const;

    std::shared_ptr<where_clause_tree_node> node;
  };

public:
  query_selector();
  query_selector(const query_selector &rhs) = default;
  query_selector &where(const where_clause &clause);
  query_selector &where(const std::string &col, const std::string &op,
                        const std::string &val);
  query_selector &where(const std::string &col, const std::string &op,
                        const std::int32_t &val);
  query_selector &where(const std::string &col, const std::string &op,
                        const std::uint32_t &val);
  query_selector &where(const std::shared_ptr<where_clause_tree_node> &root);
  query_selector &order_by(const std::string &col, order_dir dir);
  query_selector &limit(std::size_t limit);
  query_selector &offset(std::size_t offset);
  query_selector &select(const std::string &col_name);
  query_selector &select(const std::vector<std::string> &col_names);
  query_selector &relation(const std::string &rel_key);
  query_selector &relation(const std::vector<std::string> &rel_keys);

  static std::shared_ptr<where_clause_tree_node>
  or_(const where_clause_tree_node_helper &left,
      const where_clause_tree_node_helper &right);
};

} // namespace neptune

#endif // NEPTUNEORM_QUERY_SELECTOR_HPP

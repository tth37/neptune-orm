#include "neptune/query_selector.hpp"
#include "neptune/utils/exception.hpp"
#include <utility>

// =============================================================================
// neptune::query_selector =====================================================
// =============================================================================

neptune::query_selector::query_selector()
    : m_has_limit(false), m_has_offset(false), m_limit(0), m_offset(0) {}

neptune::query_selector &
neptune::query_selector::where(const query_selector::where_clause &clause) {
  auto helper = where_clause_tree_node_helper(clause);
  auto node = helper.get();
  if (m_where_clause_root == nullptr) {
    m_where_clause_root = std::move(node);
  } else {
    auto new_root = std::make_shared<where_clause_tree_node>(
        "AND", where_clause(), m_where_clause_root, node);
    m_where_clause_root = std::move(new_root);
  }
  return *this;
}

neptune::query_selector &
neptune::query_selector::where(const std::string &col, const std::string &op,
                               const std::string &val) {
  auto clause = where_clause(col, op, val);
  return where(clause);
}

neptune::query_selector &
neptune::query_selector::where(const std::string &col, const std::string &op,
                               const std::int32_t &val) {
  auto clause = where_clause(col, op, val);
  return where(clause);
}

neptune::query_selector &
neptune::query_selector::where(const std::string &col, const std::string &op,
                               const std::uint32_t &val) {
  auto clause = where_clause(col, op, val);
  return where(clause);
}

neptune::query_selector &neptune::query_selector::where(
    const std::shared_ptr<where_clause_tree_node> &root) {
  auto helper = where_clause_tree_node_helper(root);
  auto node = helper.get();
  if (m_where_clause_root == nullptr) {
    m_where_clause_root = std::move(node);
  } else {
    auto new_root = std::make_shared<where_clause_tree_node>(
        "AND", where_clause(), m_where_clause_root, node);
    m_where_clause_root = std::move(new_root);
  }
  return *this;
}

neptune::query_selector &
neptune::query_selector::order_by(const std::string &col,
                                  neptune::order_dir dir) {
  m_order_by_clauses.emplace_back(col, dir);
  return *this;
}

neptune::query_selector &neptune::query_selector::limit(std::size_t limit) {
  m_has_limit = true;
  m_limit = limit;
  return *this;
}

neptune::query_selector &neptune::query_selector::offset(std::size_t offset) {
  m_has_offset = true;
  m_offset = offset;
  return *this;
}

neptune::query_selector &
neptune::query_selector::select(const std::string &col_name) {
  m_select_cols.emplace(col_name);
  return *this;
}

neptune::query_selector &
neptune::query_selector::select(const std::vector<std::string> &col_names) {
  for (const auto &col_name : col_names) {
    m_select_cols.emplace(col_name);
  }
  return *this;
}

neptune::query_selector &
neptune::query_selector::relation(const std::string &rel_key) {
  m_select_rels.emplace(rel_key);
  return *this;
}

neptune::query_selector &
neptune::query_selector::relation(const std::vector<std::string> &rel_keys) {
  for (const auto &rel_key : rel_keys) {
    m_select_rels.emplace(rel_key);
  }
  return *this;
}

std::shared_ptr<neptune::query_selector::where_clause_tree_node>
neptune::query_selector::or_(
    const neptune::query_selector::where_clause_tree_node_helper &left,
    const neptune::query_selector::where_clause_tree_node_helper &right) {
  auto left_node = left.get();
  auto right_node = right.get();
  return std::make_shared<where_clause_tree_node>("OR", where_clause(),
                                                  left_node, right_node);
}

// =============================================================================
// neptune::query_selector::where_clause =======================================
// =============================================================================

neptune::query_selector::where_clause::where_clause(std::string col_,
                                                    std::string op_,
                                                    std::string val_)
    : col(std::move(col_)), op(std::move(op_)),
      val("\"" + std::move(val_) + "\"") {}

neptune::query_selector::where_clause::where_clause(std::string col_,
                                                    std::string op_,
                                                    std::int32_t val_)
    : col(std::move(col_)), op(std::move(op_)), val(std::to_string(val_)) {}

neptune::query_selector::where_clause::where_clause(std::string col_,
                                                    std::string op_,
                                                    std::uint32_t val_)
    : col(std::move(col_)), op(std::move(op_)), val(std::to_string(val_)) {}

neptune::query_selector::where_clause::where_clause()
    : col(""), op(""), val("") {}

// =============================================================================
// neptune::query_selector::where_clause_tree_node =============================
// =============================================================================

neptune::query_selector::where_clause_tree_node::where_clause_tree_node(
    std::string op_, neptune::query_selector::where_clause clause,
    std::shared_ptr<where_clause_tree_node> left_,
    std::shared_ptr<where_clause_tree_node> right_)
    : op(std::move(op_)), clause(std::move(clause)), left(std::move(left_)),
      right(std::move(right_)) {}

// =============================================================================
// neptune::query_selector::order_by_clause ====================================
// =============================================================================

neptune::query_selector::order_by_clause::order_by_clause(
    std::string col_, neptune::order_dir dir_)
    : col(std::move(col_)), dir(dir_) {}

// =============================================================================
// neptune::query_selector::where_clause_tree_node_helper ======================
// =============================================================================

neptune::query_selector::where_clause_tree_node_helper::
    where_clause_tree_node_helper(std::shared_ptr<where_clause_tree_node> node_)
    : node(std::move(node_)) {}

neptune::query_selector::where_clause_tree_node_helper::
    where_clause_tree_node_helper(
        const neptune::query_selector::where_clause &clause_)
    : node(std::make_shared<where_clause_tree_node>("", clause_, nullptr,
                                                    nullptr)) {}

neptune::query_selector::where_clause_tree_node_helper::
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::string val_)
    : node(std::make_shared<where_clause_tree_node>(
          "",
          neptune::query_selector::where_clause(std::move(col_), std::move(op_),
                                                std::move(val_)),
          nullptr, nullptr)) {}

neptune::query_selector::where_clause_tree_node_helper::
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::int32_t val_)
    : node(std::make_shared<where_clause_tree_node>(
          "",
          neptune::query_selector::where_clause(std::move(col_), std::move(op_),
                                                val_),
          nullptr, nullptr)) {}

neptune::query_selector::where_clause_tree_node_helper::
    where_clause_tree_node_helper(std::string col_, std::string op_,
                                  std::uint32_t val_)
    : node(std::make_shared<where_clause_tree_node>(
          "",
          neptune::query_selector::where_clause(std::move(col_), std::move(op_),
                                                val_),
          nullptr, nullptr)) {}

std::shared_ptr<neptune::query_selector::where_clause_tree_node>
neptune::query_selector::where_clause_tree_node_helper::get() const {
  return node;
}

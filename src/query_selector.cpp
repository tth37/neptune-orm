#include "neptune/query_selector.hpp"
#include "neptune/utils/exception.hpp"

// =============================================================================
// neptune::query_selector::_where_clause ======================================
// =============================================================================

neptune::query_selector::_where_clause::_where_clause(std::string col_,
                                                      std::string op_,
                                                      std::string val_)
    : col(std::move(col_)), op(std::move(op_)),
      val("\"" + std::move(val_) + "\"") {}

neptune::query_selector::_where_clause::_where_clause(std::string col_,
                                                      std::string op_,
                                                      int32_t val_)
    : col(std::move(col_)), op(std::move(op_)), val(std::to_string(val_)) {}

neptune::query_selector::_where_clause::_where_clause()
    : _where_clause("", "", "") {}

// =============================================================================
// neptune::query_selector::_where_clause_tree_node ============================
// =============================================================================

neptune::query_selector::_where_clause_tree_node::_where_clause_tree_node()
    : op(), left(), right(), clause() {}

// =============================================================================
// neptune::query_selector =====================================================
// =============================================================================

neptune::query_selector::query_selector()
    : m_where_clause_root(nullptr), m_order_by_clauses(), m_limit(0),
      m_offset(0), m_confirm_no_where(false), m_has_limit(false),
      m_has_offset(false) {}

neptune::query_selector &neptune::query_selector::where(
    const neptune::query_selector::_where_clause &where_clause) {
  if (m_where_clause_root == nullptr) {
    m_where_clause_root = std::make_shared<_where_clause_tree_node>();
    m_where_clause_root->clause = where_clause;
  } else {
    auto new_node = std::make_shared<_where_clause_tree_node>();
    new_node->clause = where_clause;
    auto new_root = std::make_shared<_where_clause_tree_node>();
    new_root->op = "AND";
    new_root->left = m_where_clause_root;
    new_root->right = new_node;
    m_where_clause_root = new_root;
  }
  return *this;
}

neptune::query_selector &
neptune::query_selector::where(const std::string &col, const std::string &op,
                               const std::string &val) {
  return where({col, op, val});
}

neptune::query_selector &neptune::query_selector::where(const std::string &col,
                                                        const std::string &op,
                                                        int32_t val) {
  return where({col, op, val});
}

neptune::query_selector &neptune::query_selector::where(
    const std::shared_ptr<_where_clause_tree_node> &where_clause_root) {
  m_where_clause_root = where_clause_root;
  return *this;
}

neptune::query_selector &neptune::query_selector::order_by(
    neptune::query_selector::_order_by_clause order_by_clause) {
  m_order_by_clauses.push_back(std::move(order_by_clause));
  return *this;
}

neptune::query_selector &neptune::query_selector::limit(std::size_t limit) {
  m_limit = limit;
  m_has_limit = true;
  return *this;
}

neptune::query_selector &neptune::query_selector::offset(std::size_t offset) {
  m_offset = offset;
  m_has_offset = true;
  return *this;
}

neptune::query_selector &neptune::query_selector::confirm_no_where() {
  m_confirm_no_where = true;
  return *this;
}

std::shared_ptr<neptune::query_selector::_where_clause_tree_node>
neptune::query_selector::or_(
    const std::shared_ptr<query_selector::_where_clause_tree_node> &left,
    const std::shared_ptr<query_selector::_where_clause_tree_node> &right) {
  auto new_root =
      std::make_shared<neptune::query_selector::_where_clause_tree_node>();
  new_root->op = "OR";
  new_root->left = left;
  new_root->right = right;
  return new_root;
}

std::shared_ptr<neptune::query_selector::_where_clause_tree_node>
neptune::query_selector::or_(
    const std::shared_ptr<query_selector::_where_clause_tree_node> &left,
    const neptune::query_selector::_where_clause &right_where_clause) {
  auto new_node =
      std::make_shared<neptune::query_selector::_where_clause_tree_node>();
  new_node->clause = right_where_clause;
  return or_(left, new_node);
}

std::shared_ptr<neptune::query_selector::_where_clause_tree_node>
neptune::query_selector::or_(
    const neptune::query_selector::_where_clause &left_where_clause,
    const std::shared_ptr<query_selector::_where_clause_tree_node> &right) {
  auto new_node =
      std::make_shared<neptune::query_selector::_where_clause_tree_node>();
  new_node->clause = left_where_clause;
  return or_(new_node, right);
}

std::shared_ptr<neptune::query_selector::_where_clause_tree_node>
neptune::query_selector::or_(
    const neptune::query_selector::_where_clause &left_where_clause,
    const neptune::query_selector::_where_clause &right_where_clause) {
  auto new_node =
      std::make_shared<neptune::query_selector::_where_clause_tree_node>();
  new_node->clause = right_where_clause;
  return or_(left_where_clause, new_node);
}

std::string neptune::query_selector::_dfs_parse_where_clause_tree(
    const std::shared_ptr<_where_clause_tree_node> &node,
    const std::set<std::string> &col_names) {
  std::string res;
  if (node->left == nullptr && node->right == nullptr) {
    res += "`" + node->clause.col + "` " + node->clause.op + " " +
           node->clause.val;
    if (col_names.find(node->clause.col) == col_names.end()) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Invalid column name in query_selector: [" +
                          node->clause.col + "]");
    }
    if (node->clause.op != "=" && node->clause.op != "!=" &&
        node->clause.op != ">" && node->clause.op != "<" &&
        node->clause.op != ">=" && node->clause.op != "<=") {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Invalid operator in query_selector: [" +
                          node->clause.op + "]");
    }
  } else {
    res += "(";
    res += _dfs_parse_where_clause_tree(node->left, col_names);
    res += " " + node->op + " ";
    res += _dfs_parse_where_clause_tree(node->right, col_names);
    res += ")";
    if (node->op != "AND" && node->op != "OR" && node->op != "and" &&
        node->op != "or") {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Invalid logic operator in query_selector: [" + node->op +
                          "]");
    }
  }
  return res;
}

std::string
neptune::query_selector::parse_where(const std::shared_ptr<entity> &e) {
  std::set<std::string> col_names;
  for (const auto &col_meta : e->get_col_metas()) {
    col_names.insert(col_meta.name);
  }
  std::string res;
  if (m_where_clause_root != nullptr) {
    res += " WHERE ";
    res += _dfs_parse_where_clause_tree(m_where_clause_root, col_names);
  } else {
    if (!m_confirm_no_where) {
      __NEPTUNE_LOG(warn, "You should explicitly specify confirm_no_where when "
                          "updating or removing without where clause");
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "No where clause in query_selector");
    }
  }
  res += ";";
  __NEPTUNE_LOG(debug, "Parsed where: " + res);
  return res;
}

std::string
neptune::query_selector::parse_query(const std::shared_ptr<entity> &e) {
  std::set<std::string> col_names;
  for (const auto &col_meta : e->get_col_metas()) {
    col_names.insert(col_meta.name);
  }
  std::string res;

  if (m_where_clause_root != nullptr) {
    res += " WHERE ";
    res += _dfs_parse_where_clause_tree(m_where_clause_root, col_names);
  }

  if (!m_order_by_clauses.empty()) {
    res += " ORDER BY ";
    for (std::size_t i = 0; i < m_order_by_clauses.size(); ++i) {
      auto &order_by_clause = m_order_by_clauses[i];
      if (i != 0) {
        res += ", ";
      }
      res += order_by_clause.col + " " + order_by_clause.dir;
      if (col_names.find(order_by_clause.col) == col_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid column name in query_selector: [" +
                            order_by_clause.col + "]");
      }
      if (order_by_clause.dir != "ASC" && order_by_clause.dir != "DESC") {
        __NEPTUNE_LOG(
            warn,
            "Only \"ASC\" and \"DESC\" are supported for order by clause");
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid direction in query_selector: [" +
                            order_by_clause.dir + "]");
      }
    }
  }

  if (m_has_limit) {
    res += " LIMIT " + std::to_string(m_limit);
  }

  if (m_has_offset) {
    res += " OFFSET " + std::to_string(m_offset);
  }

  res += ";";
  __NEPTUNE_LOG(debug, "Parsed query: " + res);
  return res;
}

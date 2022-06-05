#include "neptune/utils/parser.hpp"

std::vector<std::string> neptune::parser::create_tables(
    const std::vector<std::shared_ptr<entity>> &entities) {
  std::vector<std::string> res;
  for (const auto &e : entities) {
    std::string sql;
    sql += "CREATE TABLE IF NOT EXISTS `" + e->get_table_name() + "` (";
    bool is_first = true;
    for (const auto &col_meta : e->iter_col_metas()) {
      if (is_first)
        is_first = false;
      else
        sql += ", ";
      sql += "`" + col_meta.name + "` " + col_meta.datatype;
    }
    for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
      if (rel_1to1_meta.dir == right)
        continue;
      if (is_first)
        is_first = false;
      else
        sql += ", ";
      sql += "`" + rel_1to1_meta.key + "` VARCHAR(36)";
    }
    sql += ")";
    res.push_back(sql);
  }
  return res;
}

std::set<std::string>
neptune::parser::get_default_select_set(const std::shared_ptr<entity> &e) {
  std::set<std::string> res;
  for (const auto &col_meta : e->iter_col_metas()) {
    res.insert(col_meta.name);
  }
  return res;
}

std::set<std::string>
neptune::parser::get_select_set(const std::shared_ptr<entity> &e,
                                const query_selector &selector) {
  std::set<std::string> res;
  for (const auto &col_meta : e->iter_col_metas()) {
    if (selector.m_select_cols.empty() ||
        selector.m_select_cols.find(col_meta.name) !=
            selector.m_select_cols.end())
      res.insert(col_meta.name);
  }
  for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
    if (selector.m_select_rels.find(rel_1to1_meta.key) !=
        selector.m_select_rels.end())
      res.insert(rel_1to1_meta.key);
  }
  return res;
}

std::string neptune::parser::insert_entity(const std::shared_ptr<entity> &e) {
  // check not nullable columns
  for (const auto &col_meta : e->iter_col_metas()) {
    if (col_meta.is_nullable || col_meta.is_primary)
      continue;
    if (e->is_col_data_undefined(col_meta.name) ||
        e->is_col_data_null(col_meta.name))
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Column [" + col_meta.name + "] is not nullable");
  }

  // construct sql string
  std::string sql = "INSERT INTO `" + e->get_table_name() + "` (";
  bool is_first = true;
  for (const auto &col_meta : e->iter_col_metas()) {
    if (e->is_col_data_undefined(col_meta.name))
      continue;
    if (is_first)
      is_first = false;
    else
      sql += ", ";
    sql += "`" + col_meta.name + "`";
  }
  sql += ") VALUES (";
  is_first = true;
  for (const auto &col_meta : e->iter_col_metas()) {
    if (e->is_col_data_undefined(col_meta.name))
      continue;
    if (is_first)
      is_first = false;
    else
      sql += ", ";
    sql += e->get_col_data_as_string(col_meta.name);
  }
  sql += ")";

  return sql;
}

std::string
neptune::parser::query_last_insert_entity(const std::shared_ptr<entity> &e,
                                          const std::string &uuid) {
  // construct sql string
  std::string sql = "SELECT * FROM `" + e->get_table_name() +
                    "` WHERE `__protected_uuid` = '" + uuid + "'";

  return sql;
}

std::string neptune::parser::load_1to1_relation(
    const std::string &table, const std::string &key,
    const std::string &foreign_table, const std::string &foreign_key) {
  // construct sql string
  std::string sql = "SELECT * FROM `" + foreign_table + "` INNER JOIN `" +
                    table + "` ON `" + foreign_table + "`.`" + foreign_key +
                    "` = `" + table + "`.`" + key + "`";
  return sql;
}

std::string neptune::parser::select_entities(const std::shared_ptr<entity> &e,
                                             const query_selector &selector) {
  auto select_set = get_select_set(e, selector);

  std::set<std::string> col_names;
  for (const auto &col_meta : e->iter_col_metas()) {
    col_names.insert(col_meta.name);
  }
  std::string res = "SELECT ";
  res += select_columns(e, select_set);
  res += " FROM `" + e->get_table_name() + "`";

  if (selector.m_where_clause_root != nullptr) {
    res += " WHERE ";
    res += selector.dfs_parse_where_clause_tree(selector.m_where_clause_root,
                                                col_names);
  }

  if (!selector.m_order_by_clauses.empty()) {
    res += " ORDER BY ";
    for (std::size_t i = 0; i < selector.m_order_by_clauses.size(); ++i) {
      auto &order_by_clause = selector.m_order_by_clauses[i];
      if (i != 0) {
        res += ", ";
      }
      res += order_by_clause.col + " " +
             (order_by_clause.dir == asc ? "ASC" : "DESC");
      if (col_names.find(order_by_clause.col) == col_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid column name in query_selector: [" +
                            order_by_clause.col + "]");
      }
    }
  }

  if (selector.m_has_limit) {
    res += " LIMIT " + std::to_string(selector.m_limit);
  }

  if (selector.m_has_offset) {
    res += " OFFSET " + std::to_string(selector.m_offset);
  }

  return res;
}

std::string
neptune::parser::select_columns(const std::shared_ptr<entity> &e,
                                const std::set<std::string> &select_set) {
  if (select_set.empty()) {
    return "*";
  }

  // construct sql string
  std::string res;
  bool is_first = true;
  for (const auto &col_meta : e->iter_col_metas()) {
    if (select_set.find(col_meta.name) != select_set.end()) {
      if (is_first) {
        is_first = false;
      } else {
        res += ", ";
      }
      res += "`" + col_meta.name + "`";
    }
  }
  for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
    if (select_set.find(rel_1to1_meta.key) != select_set.end()) {
      if (rel_1to1_meta.dir == right)
        continue;
      if (is_first) {
        is_first = false;
      } else {
        res += ", ";
      }
      res += "`" + rel_1to1_meta.key + "`";
    }
  }

  return res;
}

std::vector<std::string>
neptune::parser::update_relations(const std::shared_ptr<entity> &e) {}
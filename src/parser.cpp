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
    if (selector.m_select_cols.find(rel_1to1_meta.key) !=
        selector.m_select_cols.end())
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

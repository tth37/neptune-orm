#include "neptune/connection.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

// =============================================================================
// neptune::mariadb_connection =================================================
// =============================================================================

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : m_conn(std::move(conn)) {}

void neptune::mariadb_connection::exec(const std::string &sql) {
  try {
    auto stmt = m_conn->createStatement();
    __NEPTUNE_LOG(debug, "Executing SQL: {" + sql + "}");
    stmt->execute(sql);
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::mariadb_connection::fetch(
    const std::string &sql, std::function<std::shared_ptr<entity>()> duplicate,
    const std::set<std::string> &select_set) {
  try {
    auto stmt = m_conn->createStatement();
    __NEPTUNE_LOG(debug, "Fetching SQL: {" + sql + "}");
    auto res = stmt->executeQuery(sql);
    std::vector<std::shared_ptr<neptune::entity>> ret;
    while (res->next()) {
      auto e = duplicate();
      // load columns
      for (const auto &col_meta : e->iter_col_metas()) {
        if (select_set.find(col_meta.name) == select_set.end()) {
          continue;
        }
        std::string value = (std::string)res->getString(col_meta.name);
        if (value.empty())
          e->set_col_data_null(col_meta.name);
        else
          e->set_col_data_from_string(col_meta.name, value);
      }
      // load 1-to-1 relations
      for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
        if (select_set.find(rel_1to1_meta.key) == select_set.end()) {
          continue;
        }
        std::string foreign_uuid =
            (std::string)res->getString(rel_1to1_meta.key);
        std::string load_1to1_relation_sql;
        if (rel_1to1_meta.dir == left) {
          load_1to1_relation_sql = parser::load_1to1_relation(
              e->get_table_name(), rel_1to1_meta.key,
              rel_1to1_meta.foreign_table, "__protected_uuid");
        } else {
          load_1to1_relation_sql = parser::load_1to1_relation(
              e->get_table_name(), "__protected_uuid",
              rel_1to1_meta.foreign_table, rel_1to1_meta.foreign_key);
        }
        auto foreign_entities = fetch(load_1to1_relation_sql, duplicate,
                                      parser::get_default_select_set(e));
        if (foreign_entities.empty()) {
          e->set_rel_1to1_data_null(rel_1to1_meta.key);
        } else {
          e->set_rel_1to1_data_from_entity(rel_1to1_meta.key,
                                           foreign_entities[0]);
        }
      }
      ret.push_back(e);
    }
    return ret;
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

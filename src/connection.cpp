#include "neptune/connection.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

// =============================================================================
// neptune::connection =========================================================
// =============================================================================

neptune::connection::connection() : m_mutex(), m_should_close(false) {}

neptune::query_selector neptune::connection::query() { return {}; }

std::string
neptune::connection::parse_insert_entity_sql(const std::shared_ptr<entity> &e) {
  // check not nullable columns
  for (const auto &col_meta : e->get_col_metas()) {
    if (col_meta.nullable || col_meta.is_primary)
      continue;
    if (e->is_undefined(col_meta.name) || e->is_null(col_meta.name)) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "column " + col_meta.name + " is not nullable");
    }
  }

  // construct sql string
  std::string sql = "INSERT INTO `" + e->get_table_name() + "` (";
  bool is_first = true;
  for (const auto &col_meta : e->get_col_metas()) {
    if (e->is_undefined(col_meta.name))
      continue;
    if (is_first) {
      is_first = false;
    } else {
      sql += ", ";
    }
    sql += "`" + col_meta.name + "`";
  }
  sql += ") VALUES (";
  is_first = true;
  for (const auto &col_meta : e->get_col_metas()) {
    if (e->is_undefined(col_meta.name))
      continue;
    if (is_first) {
      is_first = false;
    } else {
      sql += ", ";
    }
    sql += e->get_col_data_as_string(col_meta.name);
  }
  sql += ")";
  return sql;
}

std::string neptune::connection::parse_query_last_insert_entity_sql(
    const std::shared_ptr<entity> &e, const std::string &uuid) {
  // construct sql string
  std::string sql = "SELECT * FROM `" + e->get_table_name() +
                    "` WHERE `__protected_uuid` = '" + uuid + "'";
  return sql;
}

std::string
neptune::connection::parse_select_entities_sql(const std::shared_ptr<entity> &e,
                                               const query_selector &selector) {
  // parse select sql
  std::string select_sql = selector.parse_select_cols(e);

  // parse query sql
  std::string query_sql = selector.parse_query(e);

  // construct sql string
  std::string sql = "SELECT " + select_sql + " FROM `" + e->get_table_name() +
                    "` " + query_sql;
  return sql;
}

std::string
neptune::connection::parse_update_entity_sql(const std::shared_ptr<entity> &e) {
  // check primary column
  std::string primary_col_name = e->get_primary_col_name();
  if (e->is_undefined(primary_col_name) || e->is_null(primary_col_name)) {
    __NEPTUNE_THROW(exception_type::invalid_argument,
                    "primary column [" + primary_col_name +
                        "] is null or undefined");
  }

  // construct sql string
  std::string sql = "UPDATE `" + e->get_table_name() + "` SET ";
  bool is_first = true;
  for (const auto &col_meta : e->get_col_metas()) {
    if (e->is_undefined(col_meta.name))
      continue;
    if (is_first) {
      is_first = false;
    } else {
      sql += ", ";
    }
    sql +=
        "`" + col_meta.name + "` = " + e->get_col_data_as_string(col_meta.name);
  }
  sql += " WHERE `" + primary_col_name +
         "` = " + e->get_col_data_as_string(primary_col_name);

  return sql;
}

std::string
neptune::connection::parse_remove_entity_sql(const std::shared_ptr<entity> &e) {
  // check primary column
  std::string primary_col_name = e->get_primary_col_name();
  if (e->is_undefined(primary_col_name) || e->is_null(primary_col_name)) {
    __NEPTUNE_THROW(exception_type::invalid_argument,
                    "primary column [" + primary_col_name +
                        "] is null or undefined");
  }

  // construct sql string
  std::string sql = "DELETE FROM `" + e->get_table_name() + "` WHERE `" +
                    primary_col_name +
                    "` = " + e->get_col_data_as_string(primary_col_name);

  return sql;
}

// =============================================================================
// neptune::mariadb_connection =================================================
// =============================================================================

neptune::mariadb_connection::mariadb_connection(
    std::shared_ptr<sql::Connection> conn)
    : connection(), m_conn(std::move(conn)) {}

neptune::mariadb_connection::~mariadb_connection() {
  if (m_should_close) {
    __NEPTUNE_LOG(warn, "Connection already closed");
  }
  m_should_close = true;
  std::unique_lock<std::mutex> lock(m_mutex);
  m_conn->close();
}

void neptune::mariadb_connection::execute(const std::string &sql) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mutex);
  try {
    auto stmt = m_conn->createStatement();
    __NEPTUNE_LOG(debug, "Executing SQL: {" + sql + "}");
    stmt->execute(sql);
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::mariadb_connection::execute(
    const std::string &sql, std::function<std::shared_ptr<entity>()> duplicate,
    const std::set<std::string> &select_cols) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mutex);
  try {
    auto stmt = m_conn->createStatement();
    __NEPTUNE_LOG(debug, "Executing SQL: {" + sql + "}");
    auto res = stmt->executeQuery(sql);
    std::vector<std::shared_ptr<neptune::entity>> ret;
    while (res->next()) {
      auto e = duplicate();
      for (const auto &col_meta : e->get_col_metas()) {
        if (!select_cols.empty() &&
            select_cols.find(col_meta.name) == select_cols.end()) {
          continue;
        }
        std::string value = (std::string)res->getString(col_meta.name);
        if (!value.empty())
          e->set_col_data_from_string(col_meta.name, value);
        else
          e->set_col_data_null(col_meta.name);
      }
      ret.push_back(e);
    }
    return ret;
  } catch (const sql::SQLException &err) {
    __NEPTUNE_THROW(exception_type::sql_error, err.what());
  }
}

#include "neptune/connection.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <set>
#include <utility>

// =============================================================================
// neptune::connection =========================================================
// =============================================================================

neptune::connection::connection() : m_should_close(false) {}

neptune::connection::query_selector neptune::connection::query() { return {}; }

// =============================================================================
// neptune::connection::query_selector =========================================
// =============================================================================

neptune::connection::query_selector &neptune::connection::query_selector::where(
    const neptune::connection::query_selector::_where_clause_str
        &where_clause) {
  m_where_clauses.push_back(std::move(_where_clause{
      where_clause.col, where_clause.op, "\"" + where_clause.val + "\""}));
  return *this;
}

neptune::connection::query_selector &neptune::connection::query_selector::where(
    const neptune::connection::query_selector::_where_clause_num
        &where_clause) {
  m_where_clauses.push_back(std::move(_where_clause{
      where_clause.col, where_clause.op, std::to_string(where_clause.val)}));
  return *this;
}

neptune::connection::query_selector &
neptune::connection::query_selector::order_by(
    neptune::connection::query_selector::_order_by_clause order_by_clause) {
  m_order_by_clauses.push_back(std::move(order_by_clause));
  return *this;
}

neptune::connection::query_selector &
neptune::connection::query_selector::limit(std::size_t limit) {
  m_limit = limit;
  m_has_limit = true;
  return *this;
}

neptune::connection::query_selector &
neptune::connection::query_selector::offset(std::size_t offset) {
  m_offset = offset;
  m_has_offset = true;
  return *this;
}

neptune::connection::query_selector &
neptune::connection::query_selector::confirm_no_where() {
  m_confirm_no_where = true;
  return *this;
}

neptune::connection::query_selector::query_selector()
    : m_where_clauses(), m_order_by_clauses(), m_limit(0), m_offset(0),
      m_confirm_no_where(false), m_has_limit(false), m_has_offset(false) {}

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
  std::unique_lock<std::mutex> lock(m_mtx);
  m_conn->close();
}

void neptune::mariadb_connection::insert(const neptune::entity &e) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = e.get_insert_sql_mariadb();
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

void neptune::mariadb_connection::update(
    const neptune::entity &e,
    const neptune::connection::query_selector &selector) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = e.get_update_sql_mariadb();
  sql += parse_selector_update_remove(e, selector);
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
    __NEPTUNE_LOG(debug, "update sql: " + sql);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::mariadb_connection::select_entities(
    neptune::entity &e, const neptune::connection::query_selector &selector) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = parse_selector_query(e, selector);
  __NEPTUNE_LOG(debug, "Select SQL: " + sql);
  std::vector<std::shared_ptr<neptune::entity>> res;
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
    auto res_set = stmt->getResultSet();
    while (res_set->next()) {
      auto cur_e = e.duplicate();
      for (auto col : cur_e->get_cols()) {
        std::string value =
            (std::string)res_set->getString(col->get_col_name());
        col->set_value_from_sql_mariadb(value);
      }
      res.push_back(cur_e);
    }
    return res;
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

void neptune::mariadb_connection::remove_entities(
    neptune::entity &e, const neptune::connection::query_selector &selector) {
  if (m_should_close) {
    __NEPTUNE_THROW(exception_type::runtime_error, "Connection already closed");
  }
  std::unique_lock<std::mutex> lock(m_mtx);
  std::string sql = e.get_remove_sql_mariadb();
  sql += parse_selector_update_remove(e, selector);
  try {
    auto stmt = m_conn->createStatement();
    stmt->execute(sql);
    __NEPTUNE_LOG(debug, "remove sql: " + sql);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

std::string neptune::mariadb_connection::parse_selector_query(
    const neptune::entity &e,
    const neptune::connection::query_selector &selector) {
  std::set<std::string> col_names;
  for (auto col : e.get_cols()) {
    col_names.insert(col->get_col_name());
  }
  std::string res = "SELECT * FROM `" + e.get_table_name() + "` ";

  if (!selector.m_where_clauses.empty()) {
    res += " WHERE ";
    for (std::size_t i = 0; i < selector.m_where_clauses.size(); ++i) {
      auto &where_clause = selector.m_where_clauses[i];
      if (i != 0) {
        res += " AND ";
      }
      res += where_clause.col + " " + where_clause.op + " " + where_clause.val;
      if (col_names.find(where_clause.col) == col_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid column name in query_selector: [" +
                            where_clause.col + "]");
      }
      if (where_clause.op != "=" && where_clause.op != "!=" &&
          where_clause.op != ">" && where_clause.op != "<" &&
          where_clause.op != ">=" && where_clause.op != "<=") {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid operator in query_selector: [" +
                            where_clause.op + "]");
      }
    }
  }

  if (!selector.m_order_by_clauses.empty()) {
    res += " ORDER BY ";
    for (std::size_t i = 0; i < selector.m_order_by_clauses.size(); ++i) {
      auto &order_by_clause = selector.m_order_by_clauses[i];
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

  if (selector.m_has_limit) {
    res += " LIMIT " + std::to_string(selector.m_limit);
  }

  if (selector.m_has_offset) {
    res += " OFFSET " + std::to_string(selector.m_offset);
  }

  res += ";";
  __NEPTUNE_LOG(debug, "SQL query: " + res);
  return res;
}

std::string neptune::mariadb_connection::parse_selector_update_remove(
    const neptune::entity &e,
    const neptune::connection::query_selector &selector) {
  std::set<std::string> col_names;
  for (auto col : e.get_cols()) {
    col_names.insert(col->get_col_name());
  }
  std::string res;
  if (!selector.m_where_clauses.empty()) {
    res += " WHERE ";
    for (std::size_t i = 0; i < selector.m_where_clauses.size(); ++i) {
      auto &where_clause = selector.m_where_clauses[i];
      if (i != 0) {
        res += " AND ";
      }
      res += where_clause.col + " " + where_clause.op + " " + where_clause.val;
      if (col_names.find(where_clause.col) == col_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid column name in query_selector: [" +
                            where_clause.col + "]");
      }
      if (where_clause.op != "=" && where_clause.op != "!=" &&
          where_clause.op != ">" && where_clause.op != "<" &&
          where_clause.op != ">=" && where_clause.op != "<=") {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Invalid operator in query_selector: [" +
                            where_clause.op + "]");
      }
    }
  } else {
    if (!selector.m_confirm_no_where) {
      __NEPTUNE_LOG(warn, "You should explicitly specify confirm_no_where when "
                          "updating or removing without where clause");
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "No where clause in query_selector");
    }
  }
  res += ";";
  __NEPTUNE_LOG(debug, "SQL update where clause: " + res);
  return res;
}

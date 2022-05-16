#include "neptune/driver.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>

// =============================================================================
// neptune::driver =============================================================
// =============================================================================

neptune::driver::driver(std::string db_name) : m_db_name(std::move(db_name)) {}

void neptune::driver::register_entity(const std::shared_ptr<entity> &e) {
  m_entities.push_back(e);
}

// =============================================================================
// neptune::mariadb_driver =====================================================
// =============================================================================

neptune::mariadb_driver::mariadb_driver(std::string url, std::uint32_t port,
                                        std::string user, std::string password,
                                        std::string db_name)
    : driver(std::move(db_name)), m_url(std::move(url)), m_port(port),
      m_user(std::move(user)), m_password(std::move(password)) {
  try {
    m_driver = sql::mariadb::get_driver_instance();
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what())
  }
}

void neptune::mariadb_driver::initialize() {
  try {
    __NEPTUNE_LOG(info, "Initializing mariadb_driver [" + m_db_name + "]");
    std::shared_ptr<sql::Connection> sql_conn(m_driver->connect(
        "tcp://" + m_url + ":" + std::to_string(m_port), m_user, m_password));

    // create schema if not exists
    sql::Statement *stmt(sql_conn->createStatement());
    std::string sql("CREATE DATABASE IF NOT EXISTS ");
    sql += m_db_name;
    stmt->execute(sql);

    // use schema
    sql_conn->setSchema(m_db_name);

    // check duplicated table names
    std::set<std::string> table_names;
    for (auto &e : m_entities) {
      if (table_names.find(e->get_table_name()) != table_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated table name: [" + e->get_table_name() + "]")
      }
      table_names.insert(e->get_table_name());
    }

    // check duplicated column names and primary key
    for (auto &e : m_entities) {
      std::set<std::string> column_names;
      std::uint32_t primary_key_count(0);
      for (auto &col_meta : e->get_col_metas()) {
        if (column_names.find(col_meta.name) != column_names.end()) {
          __NEPTUNE_THROW(exception_type::invalid_argument,
                          "Duplicated column name: [" + col_meta.name + "]")
        }
        column_names.insert(col_meta.name);
        if (col_meta.is_primary) {
          primary_key_count++;
        }
      }
      if (primary_key_count != 1) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Primary key count must be 1")
      }
    }

    // create tables
    for (auto &e : m_entities) {
      sql = "CREATE TABLE IF NOT EXISTS `" + e->get_table_name() + "` (";
      for (auto &col_meta : e->get_col_metas()) {
        sql += "`" + col_meta.name + "` " + col_meta.type;
        sql += ", ";
      }
      sql.pop_back();
      sql.pop_back();
      sql += ")";
      __NEPTUNE_LOG(debug, "Create table sql: [" + sql + "]");
      stmt->execute(sql);
    }

  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

std::shared_ptr<neptune::connection>
neptune::mariadb_driver::create_connection() {
  try {
    __NEPTUNE_LOG(info,
                  "Creating connection to mariadb_driver [" + m_db_name + "]");
    std::shared_ptr<sql::Connection> sql_conn(m_driver->connect(
        "tcp://" + m_url + ":" + std::to_string(m_port), m_user, m_password));
    sql_conn->setSchema(m_db_name);
    return std::make_shared<neptune::mariadb_connection>(sql_conn);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what())
  }
}

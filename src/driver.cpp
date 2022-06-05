#include "neptune/driver.hpp"
#include "neptune/utils/parser.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>

// =============================================================================
// neptune::driver =============================================================
// =============================================================================

neptune::driver::driver(std::string db_name) : m_db_name(std::move(db_name)) {}

void neptune::driver::register_entity(const std::shared_ptr<entity> &e) {
  m_entities.push_back(e);
}

void neptune::driver::check_duplicated_table_names() {
  std::set<std::string> table_names;
  for (auto &e : m_entities) {
    if (table_names.find(e->get_table_name()) != table_names.end()) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Duplicated table name: [" + e->get_table_name() + "]")
    }
    table_names.insert(e->get_table_name());
  }
}

void neptune::driver::check_duplicated_col_rel_names() {
  for (auto &e : m_entities) {
    std::set<std::string> col_rel_names;
    for (const auto &col_meta : e->iter_col_metas()) {
      if (col_rel_names.find(col_meta.name) != col_rel_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated column relation name: [" + col_meta.name +
                            "]")
      }
      col_rel_names.insert(col_meta.name);
    }
    for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
      if (col_rel_names.find(rel_1to1_meta.key) != col_rel_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated column relation name: [" +
                            rel_1to1_meta.key + "]")
      }
      col_rel_names.insert(rel_1to1_meta.key);
    }
  }
}

void neptune::driver::check_primary_key_count() {
  for (auto &e : m_entities) {
    std::uint32_t primary_key_count(0);
    for (auto &col_meta : e->iter_col_metas()) {
      if (col_meta.is_primary) {
        primary_key_count++;
      }
    }
    if (primary_key_count != 1) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Primary key count must be 1")
    }
  }
}

void neptune::driver::check_1to1_relations() {
  struct rel_meta_checker {
    std::string table, key;
    rel_dir dir;
    std::string foreign_table, foreign_key;
  };
  std::vector<rel_meta_checker> rel_meta_checkers;
  for (const auto &e : m_entities) {
    for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
      rel_meta_checkers.push_back({rel_1to1_meta.foreign_table,
                                   rel_1to1_meta.foreign_key,
                                   rel_1to1_meta.dir == left ? right : left,
                                   e->get_table_name(), rel_1to1_meta.key});
      __NEPTUNE_LOG(debug, "1to1 relation: " + rel_1to1_meta.foreign_table +
                               "." + rel_1to1_meta.foreign_key + " -> " +
                               e->get_table_name() + "." + rel_1to1_meta.key);
    }
  }
  for (const auto &checker : rel_meta_checkers) {
    bool found = false;
    for (const auto &e : m_entities) {
      if (e->get_table_name() == checker.table) {
        for (const auto &rel_1to1_meta : e->iter_rel_1to1_metas()) {
          if (rel_1to1_meta.key == checker.key &&
              rel_1to1_meta.foreign_table == checker.foreign_table &&
              rel_1to1_meta.foreign_key == checker.foreign_key &&
              rel_1to1_meta.dir == checker.dir) {
            found = true;
            break;
          }
        }
        break;
      }
    }
    if (!found) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "1to1 relation not found: [" + checker.table + "." +
                          checker.key + "]")
    }
  }
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
    check_duplicated_table_names();

    // check duplicated column relation names
    check_duplicated_col_rel_names();

    // check primary key count
    check_primary_key_count();

    // check one_to_one relations
    check_1to1_relations();

    // create tables
    auto sqls = parser::create_tables(m_entities);
    for (const auto &create_table_sql : sqls) {
      __NEPTUNE_LOG(debug, "Create table sql: {" + create_table_sql + "}");
      stmt->execute(create_table_sql);
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

std::shared_ptr<neptune::driver> neptune::use_mariadb_driver(
    std::string url, std::uint32_t port, std::string user, std::string password,
    std::string db_name, const std::vector<std::shared_ptr<entity>> &entities) {
  auto driver = std::make_shared<neptune::mariadb_driver>(
      std::move(url), port, std::move(user), std::move(password),
      std::move(db_name));
  for (auto &e : entities) {
    driver->register_entity(e);
  }
  driver->initialize();
  return driver;
}

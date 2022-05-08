#include "neptune/driver.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>
#include <utility>

// =============================================================================
// neptune::driver =============================================================
// =============================================================================

neptune::driver::driver(std::string db_name) : m_db_name(std::move(db_name)) {}

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

void neptune::mariadb_driver::initialize() {
  try {
    __NEPTUNE_LOG(info,
                  "Start initializing mariadb_driver [" + m_db_name + "]");
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
    std::vector<std::string> tables;
    for (auto &entity : m_entities) {
      tables.push_back(entity->get_table_name());
    }
    std::sort(tables.begin(), tables.end());
    auto last = std::unique(tables.begin(), tables.end());
    if (last != tables.end()) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Duplicated table names are found in driver [" +
                          m_db_name + "]");
    }

    // check duplicated column names and primary key
    for (auto &entity : m_entities) {
      if (!entity->check_duplicated_col_names()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated column names are found in entity [" +
                            entity->get_table_name() + "]");
      }
      if (!entity->check_primary_key()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Primary key is not properly set in entity [" +
                            entity->get_table_name() + "]");
      }
    }

    // create tables
    for (auto &entity : m_entities) {
      sql = "CREATE TABLE IF NOT EXISTS ";
      sql += entity->get_define_table_sql_mariadb();
      stmt->execute(sql);
      __NEPTUNE_LOG(debug, "Define table SQL: " + sql);
    }

    __NEPTUNE_LOG(info,
                  "Finished initializing mariadb_driver [" + m_db_name + "]");
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

neptune::mariadb_driver::~mariadb_driver() { delete m_driver; }

void neptune::mariadb_driver::register_entity(
    std::shared_ptr<neptune::entity> entity) {
  __NEPTUNE_LOG(info, "Registering entity [" + entity->get_table_name() + "]" +
                          " to [" + m_db_name + "]");
  m_entities.push_back(entity);
}

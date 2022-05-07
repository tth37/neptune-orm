#include "neptune/driver.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include "neptune/utils/typedefs.hpp"
#include <mariadb/conncpp/Exception.hpp>
#include <mariadb/conncpp/Statement.hpp>

neptune::mariadb_driver::mariadb_driver(std::string url, std::uint32_t port,
                                        std::string user, std::string password)
    : m_url(std::move(url)), m_port(port), m_user(std::move(user)),
      m_password(std::move(password)) {
  try {
    m_driver = sql::mariadb::get_driver_instance();
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what())
  }
}

std::shared_ptr<neptune::connection>
neptune::mariadb_driver::create_connection() {
  try {
    std::shared_ptr<sql::Connection> sql_conn(m_driver->connect(
        "tcp://" + m_url + ":" + std::to_string(m_port), m_user, m_password));
    return std::make_shared<neptune::mariadb_connection>(sql_conn);
  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what())
  }
}

void neptune::mariadb_driver::initialize() {
  try {
    __NEPTUNE_LOG(info, "Initializing mariadb driver");
    std::shared_ptr<sql::Connection> sql_conn(m_driver->connect(
        "tcp://" + m_url + ":" + std::to_string(m_port), m_user, m_password));

    // create schema DEFAULT_DATABASE_NAME
    sql::Statement *stmt(sql_conn->createStatement());
    std::string sql_str("CREATE DATABASE IF NOT EXISTS ");
    sql_str += DEFAULT_DATABASE_NAME;
    stmt->execute(sql_str);

    // use schema DEFAULT_DATABASE_NAME
    sql_conn->setSchema(DEFAULT_DATABASE_NAME);

  } catch (const sql::SQLException &e) {
    __NEPTUNE_THROW(exception_type::sql_error, e.what());
  }
}

neptune::mariadb_driver::~mariadb_driver() { delete m_driver; }

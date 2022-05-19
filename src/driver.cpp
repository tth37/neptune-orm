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
    for (const auto &col_meta : e->get_col_metas()) {
      if (col_rel_names.find(col_meta.name) != col_rel_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated column relation name: [" + col_meta.name +
                            "]")
      }
      col_rel_names.insert(col_meta.name);
    }
    for (const auto &rel_meta : e->get_rel_metas()) {
      if (col_rel_names.find(rel_meta.key) != col_rel_names.end()) {
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Duplicated column relation name: [" + rel_meta.key +
                            "]")
      }
      col_rel_names.insert(rel_meta.key);
    }
  }
}

void neptune::driver::check_primary_key_count() {
  for (auto &e : m_entities) {
    std::uint32_t primary_key_count(0);
    for (auto &col_meta : e->get_col_metas()) {
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

void neptune::driver::check_one_to_one_relations() {
  struct rel_meta_checker {
    std::string table, key, type;
    rel_meta_dir dir;
    std::string foreign_table, foreign_key;
  };
  std::vector<rel_meta_checker> expected_rel_metas;
  for (auto &e : m_entities) {
    for (auto &rel_meta : e->get_rel_metas()) {
      if (rel_meta.type != "one_to_one") {
        continue;
      }
      // foreign entity must have a one_to_one relation respectively
      expected_rel_metas.push_back({rel_meta.foreign_table,
                                    rel_meta.foreign_key, "one_to_one",
                                    rel_meta.dir == left ? right : left,
                                    e->get_table_name(), rel_meta.key});
    }
  }
  for (auto &ck : expected_rel_metas) {
    bool found = false;
    for (auto &e : m_entities) {
      if (e->get_table_name() == ck.table) {
        for (auto &rel_meta : e->get_rel_metas()) {
          if (rel_meta.foreign_table == ck.foreign_table &&
              rel_meta.foreign_key == ck.foreign_key &&
              rel_meta.type == "one_to_one" && rel_meta.dir == ck.dir) {
            found = true;
            break;
          }
        }
        if (found) {
          break;
        }
      }
    }
    if (!found) {
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "One to one relation not found: [" + ck.table + "]")
    }
  }
}

std::string
neptune::driver::parse_create_table_sql(const std::shared_ptr<entity> &e) {
  std::string sql;
  sql += "CREATE TABLE IF NOT EXISTS `" + e->get_table_name() + "` (";
  bool is_first = true;
  for (const auto &col_meta : e->get_col_metas()) {
    if (is_first) {
      is_first = false;
    } else {
      sql += ", ";
    }
    sql += "`" + col_meta.name + "` " + col_meta.type;
  }
  for (const auto &rel_meta : e->get_rel_metas()) {
    if (rel_meta.type != "one_to_one" && rel_meta.type != "many_to_one") {
      continue;
    }
    if (is_first) {
      is_first = false;
    } else {
      sql += ", ";
    }
    sql += "`" + rel_meta.key + "` VARCHAR(36)";
  }
  sql += ")";
  return sql;
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
    check_one_to_one_relations();

    // create tables
    for (auto &e : m_entities) {
      sql = parse_create_table_sql(e);
      __NEPTUNE_LOG(debug, "Create table sql: {" + sql + "}");
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

std::shared_ptr<neptune::driver> neptune::use_mariadb_driver(
    std::string url, std::uint32_t port, std::string user, std::string password,
    std::string db_name, std::vector<std::shared_ptr<entity>> entities) {
  auto driver = std::make_shared<neptune::mariadb_driver>(url, port, user,
                                                          password, db_name);
  for (auto &e : entities) {
    driver->register_entity(e);
  }
  driver->initialize();
  return driver;
}

#include "neptune/entity.hpp"
#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

// =============================================================================
// neptune::entity =============================================================
// =============================================================================

neptune::entity::entity(std::string table_name)
    : m_table_name(std::move(table_name)) {}

const std::vector<neptune::entity::column *> &
neptune::entity::get_cols() const {
  return m_cols;
}

std::string neptune::entity::get_table_name() const { return m_table_name; }

std::string neptune::entity::get_define_table_sql_mariadb() const {
  bool is_first = true;
  std::string res = "`" + m_table_name + "` (";
  for (auto &col : m_cols) {
    if (is_first) {
      is_first = false;
    } else {
      res += ", ";
    }
    res += col->get_define_table_col_sql_mariadb();
  }
  res += ") ENGINE=InnoDB DEFAULT CHARSET=utf8;";
  return res;
}

std::string neptune::entity::get_insert_sql_mariadb() const {
  bool is_first = true;
  std::string res = "INSERT INTO `" + m_table_name + "` (";
  for (auto &col : m_cols) {
    if (is_first) {
      is_first = false;
    } else {
      res += ", ";
    }
    if (!col->is_undefined()) {
      res += "`" + col->get_col_name() + "`";
    }
  }
  res += ") VALUES (";
  is_first = true;
  for (auto &col : m_cols) {
    if (!col->is_undefined()) {
      if (is_first) {
        is_first = false;
      } else {
        res += ", ";
      }
      res += col->get_insert_col_value_sql_mariadb();
    }
  }
  res += ");";
  return res;
}

std::string neptune::entity::get_update_sql_mariadb() const {
  bool is_first = true;
  std::string res = "UPDATE `" + m_table_name + "` SET ";
  for (auto &col : m_cols) {
    if (!col->is_undefined()) {
      if (is_first) {
        is_first = false;
      } else {
        res += ", ";
      }
      res += "`" + col->get_col_name() +
             "` = " + col->get_insert_col_value_sql_mariadb();
    }
  }
  return res;
}

std::string neptune::entity::get_remove_sql_mariadb() const {
  std::string res = "DELETE FROM `" + m_table_name + "` ";
  return res;
}

bool neptune::entity::check_duplicated_col_names() const {
  std::vector<std::string> col_names;
  for (auto &col : m_cols) {
    col_names.push_back(col->get_col_name());
  }
  std::sort(col_names.begin(), col_names.end());
  auto last = std::unique(col_names.begin(), col_names.end());
  return last == col_names.end();
}

bool neptune::entity::check_primary_key() const {
  std::size_t primary_key_count = 0;
  for (auto &col : m_cols) {
    if (col->is_primary()) {
      primary_key_count++;
    }
  }
  return primary_key_count == 1;
}

void neptune::entity::define_column(neptune::entity::column &col) {
  m_cols.push_back(&col);
}

// =============================================================================
// neptune::entity::column =====================================================
// =============================================================================

neptune::entity::column::column(std::string col_name, bool nullable,
                                bool is_primary)
    : m_col_name(std::move(col_name)), m_nullable(nullable),
      m_is_primary(is_primary), m_is_null(true), m_is_undefined(true) {}

std::string neptune::entity::column::get_col_name() const { return m_col_name; }

bool neptune::entity::column::is_null() const { return m_is_null; }

bool neptune::entity::column::is_undefined() const { return m_is_undefined; }

bool neptune::entity::column::is_primary() const { return m_is_primary; }

// =============================================================================
// neptune::entity::column_int32 ===============================================
// =============================================================================

neptune::entity::column_int32::column_int32(std::string col_name, bool nullable)
    : column(std::move(col_name), nullable, false), m_value(0) {}

std::string
neptune::entity::column_int32::get_define_table_col_sql_mariadb() const {
  std::string res = "`" + m_col_name + "`";
  res += " INT";
  if (!m_nullable) {
    res += " NOT NULL";
  }
  return res;
}

std::string
neptune::entity::column_int32::get_insert_col_value_sql_mariadb() const {
  if (m_is_null) {
    return "NULL";
  } else {
    return std::to_string(m_value);
  }
}

void neptune::entity::column_int32::set_value_from_sql_mariadb(
    const std::string &value) {
  if (value == "NULL") {
    set_null();
  } else {
    try {
      set_value(std::stoi(value));
    } catch (const std::exception &e) {
      set_null();
      __NEPTUNE_LOG(warn, "Failed to set value from sql_mariadb \"" + value +
                              "\" at column [" + m_col_name + "]");
    }
  }
}

void neptune::entity::column_int32::set_null() {
  m_is_null = true;
  m_is_undefined = false;
  m_value = 0;
}

int32_t neptune::entity::column_int32::value() const { return m_value; }

void neptune::entity::column_int32::set_value(int32_t value) {
  m_is_null = false;
  m_is_undefined = false;
  m_value = value;
}

neptune::entity::column_int32 &
neptune::entity::column_int32::operator=(int32_t value) {
  set_value(value);
  return *this;
}

// =============================================================================
// neptune::entity::column_primary_generated_uint32 ============================
// =============================================================================

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(std::string col_name)
    : column(std::move(col_name), false, true), m_value(0) {}

std::string neptune::entity::column_primary_generated_uint32::
    get_define_table_col_sql_mariadb() const {
  std::string res = "`" + m_col_name + "`";
  res += " INT UNSIGNED AUTO_INCREMENT PRIMARY KEY";
  return res;
}

std::string neptune::entity::column_primary_generated_uint32::
    get_insert_col_value_sql_mariadb() const {
  if (m_is_null) {
    return "NULL";
  } else {
    return std::to_string(m_value);
  }
}

void neptune::entity::column_primary_generated_uint32::
    set_value_from_sql_mariadb(const std::string &value) {
  if (value == "NULL") {
    set_null();
  } else {
    try {
      set_value(std::stoul(value));
    } catch (const std::exception &e) {
      set_null();
      __NEPTUNE_LOG(warn, "Failed to set value from sql_mariadb \"" + value +
                              "\" at column [" + m_col_name + "]");
    }
  }
}

void neptune::entity::column_primary_generated_uint32::set_null() {
  m_is_null = true;
  m_is_undefined = false;
  m_value = 0;
}

uint32_t neptune::entity::column_primary_generated_uint32::value() const {
  return m_value;
}

void neptune::entity::column_primary_generated_uint32::set_value(
    uint32_t value) {
  m_is_null = false;
  m_is_undefined = false;
  m_value = value;
}

neptune::entity::column_primary_generated_uint32 &
neptune::entity::column_primary_generated_uint32::operator=(uint32_t value) {
  set_value(value);
  return *this;
}

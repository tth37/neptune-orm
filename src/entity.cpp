#include "neptune/entity.hpp"

#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

neptune::entity::entity(std::string table_name)
    : m_table_name(std::move(table_name)) {}

std::string neptune::entity::get_table_name() const { return m_table_name; }

std::string neptune::entity::get_table_meta() const {
  std::string table_meta;
  for (std::size_t i = 0; i < m_cols.size(); ++i) {
    table_meta += "`" + m_cols[i]->get_col_name() + "` ";
    table_meta += m_cols[i]->get_col_meta();
    if (i != m_cols.size() - 1) {
      table_meta += ", ";
    }
  }
  return table_meta;
}

bool neptune::entity::check_duplicated_col_names() const {
  std::vector<std::string> col_names;
  for (const auto &col : m_cols) {
    col_names.push_back(col->get_col_name());
  }
  std::sort(col_names.begin(), col_names.end());
  auto last = std::unique(col_names.begin(), col_names.end());
  return last == col_names.end();
}

bool neptune::entity::check_primary_key() const {
  std::size_t primary_key_count = 0;
  for (const auto &col : m_cols) {
    if (col->is_primary_key()) {
      ++primary_key_count;
    }
  }
  return primary_key_count == 1;
}

std::string neptune::entity::get_insert_sql() const {
  std::string sql;
  sql += "INSERT INTO `" + m_table_name + "` (";
  for (std::size_t i = 0; i < m_cols.size(); ++i) {
    sql += "`" + m_cols[i]->get_col_name() + "`";
    if (i != m_cols.size() - 1) {
      sql += ", ";
    }
  }
  sql += ") VALUES (";
  for (std::size_t i = 0; i < m_cols.size(); ++i) {
    sql += m_cols[i]->get_insert_value();
    if (i != m_cols.size() - 1) {
      sql += ", ";
    }
  }
  sql += ")";
  return sql;
}

std::vector<std::shared_ptr<neptune::entity::column>> &
neptune::entity::get_cols() {
  return m_cols;
}

neptune::entity::column::column(std::string col_name, bool nullable,
                                bool is_primary_key)
    : m_col_name(std::move(col_name)), m_nullable(nullable),
      m_is_primary_key(is_primary_key), m_is_null(true) {}

void neptune::entity::column::set_null() { m_is_null = true; }

std::string neptune::entity::column::get_col_name() const { return m_col_name; }

bool neptune::entity::column::is_primary_key() const {
  return m_is_primary_key;
}

neptune::entity::column_int32::column_int32(const std::string &col_name,
                                            bool nullable)
    : column(col_name, nullable, false), m_value(0) {}

std::string neptune::entity::column_int32::get_col_meta() const {
  if (m_nullable)
    return "INT";
  else
    return "INT NOT NULL";
}

void neptune::entity::column_int32::set_value(int32_t value) {
  __NEPTUNE_LOG(debug, "set value: " + std::to_string(value));
  m_value = value;
  m_is_null = false;
}

std::tuple<bool, int32_t> neptune::entity::column_int32::value() const {
  return std::make_tuple(m_is_null, m_value);
}

std::string neptune::entity::column_int32::get_insert_value() const {
  __NEPTUNE_LOG(debug, "column_int32::get_insert_value()" +
                           (m_is_null ? "NULL" : std::to_string(m_value)));
  if (m_is_null) {
    return "NULL";
  } else {
    return std::to_string(m_value);
  }
}

void neptune::entity::column_int32::set_value_from_string(
    const std::string &value) {
  __NEPTUNE_LOG(debug, "set_value_from_string: " + value);
  if (value == "NULL") {
    m_is_null = true;
  } else {
    m_is_null = false;
    try {
      m_value = std::stoi(value);
    } catch (...) {
      m_is_null = true;
    }
  }
}

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(const std::string &col_name)
    : column(col_name, false, true), m_value(0) {}
std::string

neptune::entity::column_primary_generated_uint32::get_col_meta() const {
  return "INT UNSIGNED AUTO_INCREMENT PRIMARY KEY";
}

std::tuple<bool, uint32_t>
neptune::entity::column_primary_generated_uint32::value() const {
  return std::make_tuple(m_is_null, m_value);
}

std::string
neptune::entity::column_primary_generated_uint32::get_insert_value() const {
  return std::to_string(m_value);
}

void neptune::entity::column_primary_generated_uint32::set_value_from_string(
    const std::string &value) {
  __NEPTUNE_LOG(debug, "id set_value_from_string: " + value);
  if (value == "NULL") {
    m_is_null = true;
  } else {
    m_is_null = false;
    try {
      m_value = std::stoul(value);
    } catch (...) {
      m_is_null = true;
    }
  }
  __NEPTUNE_LOG(debug, "id set_value_from_string: " + std::to_string(m_value));
}

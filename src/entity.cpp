#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

// =============================================================================
// neptune::entity =============================================================
// =============================================================================

neptune::entity::entity(std::string table_name)
    : m_table_name(std::move(table_name)), m_col_metas(), m_col_container() {}

void neptune::entity::set_col_data_from_string(const std::string &col_name,
                                               const std::string &value) {
  m_col_container.at(col_name)->set_value_from_string(value);
}

std::string
neptune::entity::get_col_data_as_string(const std::string &col_name) const {
  return m_col_container.at(col_name)->get_value_as_string();
}

std::string neptune::entity::get_rel_uuid(const std::string &rel_key) const {
  return std::static_pointer_cast<rel_data_single>(m_rel_container.at(rel_key))
      ->get_uuid();
}

bool neptune::entity::is_undefined_col(const std::string &col_name) const {
  return m_col_container.at(col_name)->is_undefined();
}

bool neptune::entity::is_null_col(const std::string &col_name) const {
  return m_col_container.at(col_name)->is_null();
}

bool neptune::entity::is_undefined_rel(const std::string &rel_key) const {
  return m_rel_container.at(rel_key)->is_undefined();
}

bool neptune::entity::is_null_rel(const std::string &rel_key) const {
  return m_rel_container.at(rel_key)->is_null();
}

const std::vector<neptune::entity::col_meta> &
neptune::entity::get_col_metas() const {
  return m_col_metas;
}

const std::vector<neptune::entity::rel_meta> &
neptune::entity::get_rel_metas() const {
  return m_rel_metas;
}

std::string neptune::entity::get_table_name() const { return m_table_name; }

void neptune::entity::set_col_data_null(const std::string &col_name) {
  m_col_container.at(col_name)->set_null();
}

void neptune::entity::set_col_data_undefined(const std::string &col_name) {
  m_col_container.at(col_name)->set_undefined();
}

void neptune::entity::set_rel_target_uuid(const std::string &rel_key,
                                          const std::string &target_uuid) {
  std::static_pointer_cast<rel_data_single>(m_rel_container.at(rel_key))
      ->set_target_uuid(target_uuid);
}

void neptune::entity::set_rel_data_null(const std::string &rel_key) {
  m_rel_container.at(rel_key)->set_null();
}

void neptune::entity::set_rel_data_from_entity(
    const std::string &rel_key, const std::shared_ptr<entity> &value) {
  std::static_pointer_cast<rel_data_single>(m_rel_container.at(rel_key))
      ->set_value(value);
}

std::string
neptune::entity::get_rel_target_uuid(const std::string &rel_key) const {
  return std::static_pointer_cast<rel_data_single>(m_rel_container.at(rel_key))
      ->get_target_uuid();
}

std::string neptune::entity::get_primary_col_name() const {
  for (const auto &col_meta : m_col_metas) {
    if (col_meta.is_primary) {
      return col_meta.name;
    }
  }
  __NEPTUNE_THROW(exception_type::runtime_error,
                  "No primary column found in table " + m_table_name);
}

void neptune::entity::check_expected_rel_meta(
    const std::string &rel_key, const std::string &type,
    const std::string &foreign_table, const std::string &foreign_key) const {
  for (const auto &rel_meta : m_rel_metas) {
    if (rel_meta.key == rel_key && rel_meta.type == type &&
        rel_meta.foreign_table == foreign_table &&
        rel_meta.foreign_key == foreign_key) {
      return;
    }
  }
  __NEPTUNE_THROW(exception_type::invalid_argument,
                  "No relation meta found for key [" + rel_key +
                      "] and type [" + type + "]");
}

void neptune::entity::check_expected_rel_meta(
    const std::string &rel_key, const std::string &type, rel_meta_dir dir,
    const std::string &foreign_table, const std::string &foreign_key) const {
  for (const auto &rel_meta : m_rel_metas) {
    if (rel_meta.key == rel_key && rel_meta.type == type &&
        rel_meta.dir == dir && rel_meta.foreign_table == foreign_table &&
        rel_meta.foreign_key == foreign_key) {
      return;
    }
  }
  __NEPTUNE_THROW(exception_type::invalid_argument,
                  "No relation meta found for key [" + rel_key +
                      "] and type [" + type + "]");
}

// =============================================================================
// neptune::entity::col_data ===================================================
// =============================================================================

neptune::entity::col_data::col_data()
    : m_is_null(false), m_is_undefined(true) {}

bool neptune::entity::col_data::is_null() const { return m_is_null; }

void neptune::entity::col_data::set_null() {
  m_is_null = true;
  m_is_undefined = false;
}

bool neptune::entity::col_data::is_undefined() const { return m_is_undefined; }

void neptune::entity::col_data::set_undefined() { m_is_undefined = true; }

// =============================================================================
// neptune::entity::col_data_uint32 ============================================
// =============================================================================

neptune::entity::col_data_uint32::col_data_uint32() : col_data(), m_value(0) {}

void neptune::entity::col_data_uint32::set_value_from_string(
    const std::string &value) {
  try {
    m_value = std::stoul(value);
    m_is_null = false;
    m_is_undefined = false;
  } catch (const std::exception &e) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Failed to convert SQL string to uint32: [" + value + "]");
  }
}

std::string neptune::entity::col_data_uint32::get_value_as_string() const {
  if (m_is_null) {
    return "NULL";
  } else {
    return std::to_string(m_value);
  }
}

std::uint32_t neptune::entity::col_data_uint32::get_value() const {
  return m_value;
}

void neptune::entity::col_data_uint32::set_value(std::uint32_t value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

// =============================================================================
// neptune::entity::col_data_int32 =============================================
// =============================================================================

neptune::entity::col_data_int32::col_data_int32() : col_data(), m_value(0) {}

void neptune::entity::col_data_int32::set_value_from_string(
    const std::string &value) {
  try {
    m_value = std::stoi(value);
    m_is_null = false;
    m_is_undefined = false;
  } catch (const std::exception &e) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Failed to convert SQL string to int32: [" + value + "]");
  }
}

std::string neptune::entity::col_data_int32::get_value_as_string() const {
  if (m_is_null) {
    return "NULL";
  } else {
    return std::to_string(m_value);
  }
}

std::int32_t neptune::entity::col_data_int32::get_value() const {
  return m_value;
}

void neptune::entity::col_data_int32::set_value(std::int32_t value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

// =============================================================================
// neptune::entity::col_data_string ============================================
// =============================================================================

neptune::entity::col_data_string::col_data_string() : col_data(), m_value() {}

void neptune::entity::col_data_string::set_value_from_string(
    const std::string &value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

std::string neptune::entity::col_data_string::get_value_as_string() const {
  if (m_is_null) {
    return "NULL";
  } else {
    return "\"" + m_value + "\"";
  }
}

const std::string &neptune::entity::col_data_string::get_value() const {
  return m_value;
}

void neptune::entity::col_data_string::set_value(const std::string &value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

// =============================================================================
// neptune::entity::col_meta ===================================================
// =============================================================================

neptune::entity::col_meta::col_meta(std::string name_, std::string type_,
                                    bool is_primary_, bool nullable_)
    : name(std::move(name_)), type(std::move(type_)), is_primary(is_primary_),
      nullable(nullable_) {}

// =============================================================================
// neptune::entity::rel_data ===================================================
// =============================================================================

neptune::entity::rel_data::rel_data()
    : m_is_null(false), m_is_undefined(true) {}

bool neptune::entity::rel_data::is_null() const { return m_is_null; }

void neptune::entity::rel_data::set_null() {
  m_is_null = true;
  m_is_undefined = false;
}

bool neptune::entity::rel_data::is_undefined() const { return m_is_undefined; }

void neptune::entity::rel_data::set_undefined() { m_is_undefined = true; }

// =============================================================================
// neptune::entity::rel_data_single ============================================
// =============================================================================

neptune::entity::rel_data_single::rel_data_single() : m_value() {}

void neptune::entity::rel_data_single::set_value(
    const std::shared_ptr<entity> &value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

std::string neptune::entity::rel_data_single::get_uuid() const {
  if (m_is_null) {
    return "NULL";
  } else {
    if (m_value->is_undefined_col("__protected_uuid"))
      __NEPTUNE_THROW(exception_type::invalid_argument,
                      "Relation entity not instantiated");
    return m_value->get_col_data_as_string("__protected_uuid");
  }
}

std::shared_ptr<neptune::entity>
neptune::entity::rel_data_single::get_value() const {
  return m_value;
}

void neptune::entity::rel_data_single::set_target_uuid(
    const std::string &target_uuid) {
  m_target_uuid = target_uuid;
}

std::string neptune::entity::rel_data_single::get_target_uuid() const {
  return m_target_uuid;
}

// =============================================================================
// neptune::entity::rel_data_multiple ==========================================
// =============================================================================

neptune::entity::rel_data_multiple::rel_data_multiple() : m_value() {}

void neptune::entity::rel_data_multiple::set_value(
    const std::vector<std::shared_ptr<entity>> &value) {
  m_value = value;
}

std::vector<std::string> neptune::entity::rel_data_multiple::get_uuid() const {
  if (m_is_null) {
    return {};
  } else {
    std::vector<std::string> uuids;
    for (const auto &entity : m_value) {
      if (entity->is_undefined_col("__protected_uuid"))
        __NEPTUNE_THROW(exception_type::invalid_argument,
                        "Relation entity not instantiated");
      uuids.push_back(entity->get_col_data_as_string("__protected_uuid"));
    }
    return uuids;
  }
}

std::vector<std::shared_ptr<neptune::entity>>
neptune::entity::rel_data_multiple::get_value() const {
  return m_value;
}

// =============================================================================
// neptune::entity::rel_meta ===================================================
// =============================================================================

neptune::entity::rel_meta::rel_meta(std::string key_, std::string type_,
                                    rel_meta_dir dir_,
                                    std::string foreign_table_,
                                    std::string foreign_key_)
    : key(std::move(key_)), type(std::move(type_)), dir(dir_),
      foreign_table(std::move(foreign_table_)),
      foreign_key(std::move(foreign_key_)) {}

// =============================================================================
// neptune::entity::column =====================================================
// =============================================================================

neptune::entity::column::column(neptune::entity *this_ptr, std::string col_name,
                                bool nullable, bool is_primary)
    : m_col_name(std::move(col_name)), m_nullable(nullable),
      m_is_primary(is_primary), m_container_ref(this_ptr->m_col_container),
      m_metas_ref(this_ptr->m_col_metas) {}

std::string neptune::entity::column::get_col_name() const { return m_col_name; }

bool neptune::entity::column::is_undefined() const {
  return m_container_ref.at(m_col_name)->is_undefined();
}

void neptune::entity::column::set_undefined() {
  m_container_ref.at(m_col_name)->set_undefined();
}

bool neptune::entity::column::is_null() const {
  if (is_undefined()) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Column [" + m_col_name + "] is undefined");
  } else {
    return m_container_ref.at(m_col_name)->is_null();
  }
}

void neptune::entity::column::set_null() {
  m_container_ref.at(m_col_name)->set_null();
}

neptune::entity::column::operator bool() const {
  if (is_undefined())
    return false;
  if (is_null())
    return false;
  return true;
}

void neptune::entity::column::insert_int32_to_container_if_necessary() {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    m_container_ref.emplace(m_col_name, std::make_shared<col_data_int32>());
  }
}

void neptune::entity::column::insert_uint32_to_container_if_necessary() {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    m_container_ref.emplace(m_col_name, std::make_shared<col_data_uint32>());
  }
}

void neptune::entity::column::insert_string_to_container_if_necessary() {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    m_container_ref.emplace(m_col_name, std::make_shared<col_data_string>());
  }
}

// =============================================================================
// neptune::entity::column_primary_generated_uint32 ============================
// =============================================================================

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name)
    : column(this_ptr, std::move(col_name), false, true) {
  insert_uint32_to_container_if_necessary();
  m_metas_ref.emplace_back(
      m_col_name, "INT UNSIGNED AUTO_INCREMENT PRIMARY KEY", true, false);
}

std::uint32_t
neptune::entity::column_primary_generated_uint32::get_value() const {
  if (is_undefined()) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Column [" + m_col_name + "] is undefined");
  } else {
    return std::dynamic_pointer_cast<col_data_uint32>(
               m_container_ref.at(m_col_name))
        ->get_value();
  }
}

void neptune::entity::column_primary_generated_uint32::set_value(
    std::uint32_t value) {
  std::dynamic_pointer_cast<col_data_uint32>(m_container_ref.at(m_col_name))
      ->set_value(value);
}

// =============================================================================
// neptune::entity::column_int32 ===============================================
// =============================================================================

neptune::entity::column_int32::column_int32(neptune::entity *this_ptr,
                                            std::string col_name, bool nullable)
    : column(this_ptr, std::move(col_name), nullable, false) {
  insert_int32_to_container_if_necessary();
  std::string type = "INT";
  if (!nullable) {
    type += " NOT NULL";
  }
  m_metas_ref.emplace_back(m_col_name, type, false, nullable);
}

std::int32_t neptune::entity::column_int32::get_value() const {
  if (is_undefined()) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Column [" + m_col_name + "] is undefined");
  } else {
    return std::dynamic_pointer_cast<col_data_int32>(
               m_container_ref.at(m_col_name))
        ->get_value();
  }
}

void neptune::entity::column_int32::set_value(std::int32_t value) {
  std::dynamic_pointer_cast<col_data_int32>(m_container_ref.at(m_col_name))
      ->set_value(value);
}

// =============================================================================
// neptune::entity::column_varchar =============================================
// =============================================================================

neptune::entity::column_varchar::column_varchar(neptune::entity *this_ptr,
                                                std::string col_name,
                                                bool nullable,
                                                std::size_t max_length)
    : column(this_ptr, std::move(col_name), nullable, false),
      m_max_length(max_length) {
  insert_string_to_container_if_necessary();
  std::string type = "VARCHAR(" + std::to_string(m_max_length) + ")";
  if (!nullable) {
    type += " NOT NULL";
  }
  m_metas_ref.emplace_back(m_col_name, type, false, nullable);
}

std::string neptune::entity::column_varchar::get_value() const {
  if (is_undefined()) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Column [" + m_col_name + "] is undefined");
  } else {
    return std::dynamic_pointer_cast<col_data_string>(
               m_container_ref.at(m_col_name))
        ->get_value();
  }
}

void neptune::entity::column_varchar::set_value(const std::string &value) {
  if (value.size() > m_max_length) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Column [" + m_col_name + "] is too long");
  }
  std::dynamic_pointer_cast<col_data_string>(m_container_ref.at(m_col_name))
      ->set_value(value);
}

// =============================================================================
// neptune::entity::relation ===================================================
// =============================================================================

neptune::entity::relation::relation(neptune::entity *this_ptr,
                                    std::string rel_key)
    : m_rel_key(std::move(rel_key)), m_container_ref(this_ptr->m_rel_container),
      m_metas_ref(this_ptr->m_rel_metas) {}

std::string neptune::entity::relation::get_rel_key() const { return m_rel_key; }

bool neptune::entity::relation::is_undefined() const {
  return m_container_ref.at(m_rel_key)->is_undefined();
}

void neptune::entity::relation::set_undefined() {
  m_container_ref.at(m_rel_key)->set_undefined();
}

bool neptune::entity::relation::is_null() const {
  return m_container_ref.at(m_rel_key)->is_null();
}

void neptune::entity::relation::set_null() {
  m_container_ref.at(m_rel_key)->set_null();
}

neptune::entity::relation::operator bool() const {
  if (is_undefined())
    return false;
  if (is_null())
    return false;
  return true;
}

void neptune::entity::relation::insert_single_to_container_if_necessary() {
  if (m_container_ref.find(m_rel_key) == m_container_ref.end()) {
    m_container_ref.emplace(m_rel_key, std::make_shared<rel_data_single>());
  }
}

void neptune::entity::relation::insert_multiple_to_container_if_necessary() {
  if (m_container_ref.find(m_rel_key) == m_container_ref.end()) {
    m_container_ref.emplace(m_rel_key, std::make_shared<rel_data_multiple>());
  }
}
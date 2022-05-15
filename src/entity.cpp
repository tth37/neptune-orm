#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

// =============================================================================
// neptune::entity =============================================================
// =============================================================================

neptune::entity::entity(std::string table_name)
    : m_table_name(std::move(table_name)), m_col_metas(), m_col_container(),
      _protected_id(this, "_protected_id") {}

neptune::entity::entity(const neptune::entity &e)
    : m_table_name(e.m_table_name), m_col_metas(),
      m_col_container(e.m_col_container), _protected_id(this, "_protected_id") {
}

const std::vector<neptune::entity::col_meta> &
neptune::entity::get_col_metas() const {
  return m_col_metas;
}

void neptune::entity::set_col_data_from_string(const std::string &col_name,
                                               const std::string &value) {
  m_col_container.at(col_name)->set_value_from_string(value);
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
// neptune::entity::col_meta ===================================================
// =============================================================================

neptune::entity::col_meta::col_meta(std::string name_, bool is_primary_)
    : name(std::move(name_)), is_primary(is_primary_) {}

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

// =============================================================================
// neptune::entity::column_primary_generated_uint32 ============================
// =============================================================================

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name)
    : column(this_ptr, std::move(col_name), false, true) {
  insert_uint32_to_container_if_necessary();
  m_metas_ref.emplace_back(m_col_name, true);
}

std::uint32_t
neptune::entity::column_primary_generated_uint32::get_value() const {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    __NEPTUNE_THROW(neptune::exception_type::runtime_error,
                    "column [" + m_col_name + "] is undefined");
  }
  return std::dynamic_pointer_cast<col_data_uint32>(it->second)->get_value();
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
  m_metas_ref.emplace_back(m_col_name, false);
  __NEPTUNE_LOG(debug, "Calls column_int32 constructor");
}

neptune::entity::column_int32::column_int32(
    const neptune::entity::column_int32 &c)
    : column(c) {
  insert_int32_to_container_if_necessary();
  m_metas_ref.emplace_back(m_col_name, false);
  __NEPTUNE_LOG(debug, "Calls column_int32 copy constructor");
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

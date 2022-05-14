#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

// =============================================================================
// neptune::entity =============================================================
// =============================================================================

const std::vector<std::string> &neptune::entity::get_col_names() const {
  return m_col_names;
}

// =============================================================================
// neptune::entity::col_data ===================================================
// =============================================================================

neptune::entity::col_data::col_data() : m_is_null(true) {}

bool neptune::entity::col_data::is_null() const { return m_is_null; }

void neptune::entity::col_data::set_null() { m_is_null = true; }

// =============================================================================
// neptune::entity::col_data_uint32 ============================================
// =============================================================================

neptune::entity::col_data_uint32::col_data_uint32() : col_data(), m_value(0) {}

void neptune::entity::col_data_uint32::set_value_from_string(
    const std::string &value) {
  try {
    m_value = std::stoul(value);
    m_is_null = false;
  } catch (const std::exception &e) {
    m_is_null = true;
    __NEPTUNE_LOG(error,
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
  } catch (const std::exception &e) {
    m_is_null = true;
    __NEPTUNE_LOG(error,
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
}

// =============================================================================
// neptune::entity::column =====================================================
// =============================================================================

neptune::entity::column::column(neptune::entity *this_ptr, std::string col_name,
                                bool nullable, bool is_primary)
    : m_col_name(std::move(col_name)), m_nullable(nullable),
      m_is_primary(is_primary), m_container_ref(this_ptr->m_col_container),
      m_names_ref(this_ptr->m_col_names) {}

std::string neptune::entity::column::get_col_name() const { return m_col_name; }

bool neptune::entity::column::is_undefined() const {
  auto it = m_container_ref.find(m_col_name);
  return it == m_container_ref.end();
}

void neptune::entity::column::set_undefined() {
  auto it = m_container_ref.find(m_col_name);
  if (it != m_container_ref.end()) {
    m_container_ref.erase(it);
  }
}

bool neptune::entity::column::is_null() const {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    __NEPTUNE_THROW(neptune::exception_type::runtime_error,
                    "column [" + m_col_name + "] is undefined");
  }
  return it->second->is_null();
}

void neptune::entity::column::insert_int32_to_container_if_necessary() {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    m_container_ref.emplace(m_col_name, std::make_shared<col_data_int32>());
    m_names_ref.emplace_back(m_col_name);
  }
}

void neptune::entity::column::insert_uint32_to_container_if_necessary() {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    m_container_ref.emplace(m_col_name, std::make_shared<col_data_uint32>());
    m_names_ref.emplace_back(m_col_name);
  }
}

// =============================================================================
// neptune::entity::column_primary_generated_uint32 ============================
// =============================================================================

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name)
    : column(this_ptr, std::move(col_name), false, true) {
  m_names_ref.push_back(col_name);
}

void neptune::entity::column_primary_generated_uint32::set_null() {
  insert_uint32_to_container_if_necessary();
  m_container_ref.at(m_col_name)->set_null();
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
  insert_uint32_to_container_if_necessary();
  std::dynamic_pointer_cast<col_data_uint32>(m_container_ref.at(m_col_name))
      ->set_value(value);
}

// =============================================================================
// neptune::entity::column_int32 ===============================================
// =============================================================================

neptune::entity::column_int32::column_int32(neptune::entity *this_ptr,
                                            std::string col_name, bool nullable)
    : column(this_ptr, std::move(col_name), nullable, false) {
  m_names_ref.push_back(col_name);
}

void neptune::entity::column_int32::set_null() {
  insert_int32_to_container_if_necessary();
  m_container_ref.at(m_col_name)->set_null();
}

std::int32_t neptune::entity::column_int32::get_value() const {
  auto it = m_container_ref.find(m_col_name);
  if (it == m_container_ref.end()) {
    __NEPTUNE_THROW(neptune::exception_type::runtime_error,
                    "column [" + m_col_name + "] is undefined");
  }
  return std::dynamic_pointer_cast<col_data_int32>(it->second)->get_value();
}

void neptune::entity::column_int32::set_value(std::int32_t value) {
  insert_int32_to_container_if_necessary();
  std::dynamic_pointer_cast<col_data_int32>(m_container_ref.at(m_col_name))
      ->set_value(value);
}

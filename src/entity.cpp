#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <algorithm>
#include <utility>

// =============================================================================
// neptune::entity =============================================================
// =============================================================================

neptune::entity::entity(std::string table_name)
    : m_table_name(std::move(table_name)) {}

std::string neptune::entity::get_table_name() const { return m_table_name; }

// =============================================================================
//  neptune::entity::col_data ==================================================
// =============================================================================

neptune::entity::col_data::col_data() : m_is_null(true), m_is_undefined(true) {}

bool neptune::entity::col_data::is_null() const { return m_is_null; }

void neptune::entity::col_data::set_null() {
  m_is_null = true;
  m_is_undefined = false;
}

bool neptune::entity::col_data::is_undefined() const { return m_is_undefined; }

void neptune::entity::col_data::set_undefined() { m_is_undefined = true; }

neptune::entity::col_data_uint32::col_data_uint32() : col_data(), m_value(0) {}

void neptune::entity::col_data_uint32::set_value_from_string(
    const std::string &value) {
  try {
    m_value = std::stoul(value);
    m_is_null = false;
    m_is_undefined = false;
  } catch (const std::exception &e) {
    m_is_null = true;
    m_is_undefined = false;
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Failed to convert string to uint32: [" + value + "]");
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

neptune::entity::col_data_string::col_data_string() : col_data() {}
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

std::string neptune::entity::col_data_string::get_value() const {
  return m_value;
}

void neptune::entity::col_data_string::set_value(const std::string &value) {
  m_value = value;
  m_is_null = false;
  m_is_undefined = false;
}

void neptune::entity::set_col_data_from_string(const std::string &col_name,
                                               const std::string &value) {
  m_col_container[col_name]->set_value_from_string(value);
}

void neptune::entity::set_col_data_null(const std::string &col_name) {
  m_col_container[col_name]->set_null();
}

void neptune::entity::set_col_data_undefined(const std::string &col_name) {
  m_col_container[col_name]->set_undefined();
}

std::string
neptune::entity::get_col_data_as_string(const std::string &col_name) const {
  return m_col_container.at(col_name)->get_value_as_string();
}

bool neptune::entity::is_col_data_null(const std::string &col_name) const {
  return m_col_container.at(col_name)->is_null();
}

bool neptune::entity::is_col_data_undefined(const std::string &col_name) const {
  return m_col_container.at(col_name)->is_undefined();
}

// =============================================================================
// neptune::entity::col_meta ===================================================
// =============================================================================

neptune::entity::col_meta::col_meta(std::string name_, std::string datatype_,
                                    bool is_primary_, bool is_nullable_)
    : name(std::move(name_)), datatype(std::move(datatype_)),
      is_primary(is_primary_), is_nullable(is_nullable_) {}

const std::vector<neptune::entity::col_meta> &
neptune::entity::iter_col_metas() const {
  return m_col_metas;
}

// =============================================================================
// neptune::entity::rel_data ===================================================
// =============================================================================

neptune::entity::rel_data::rel_data() : m_is_null(true), m_is_undefined(true) {}

bool neptune::entity::rel_data::is_null() const { return m_is_null; }

void neptune::entity::rel_data::set_null() {
  m_is_null = true;
  m_is_undefined = false;
}

bool neptune::entity::rel_data::is_undefined() const { return m_is_undefined; }

void neptune::entity::rel_data::set_undefined() { m_is_undefined = true; }

neptune::entity::rel_1to1_data::rel_1to1_data()
    : rel_data(), m_entity(nullptr) {}

std::shared_ptr<neptune::entity> neptune::entity::rel_1to1_data::get_entity() {
  return m_entity;
}

void neptune::entity::rel_1to1_data::set_entity(
    std::shared_ptr<entity> entity) {
  m_entity = std::move(entity);
}

void neptune::entity::set_rel_1to1_data_from_entity(
    const std::string &col_name, const std::shared_ptr<entity> &e) {
  m_rel_1to1_container[col_name]->set_entity(e);
}

void neptune::entity::set_rel_1to1_data_from_entities(
    const std::string &col_name,
    const std::vector<std::shared_ptr<entity>> &es) {
  if (es.size() != 1) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Failed to set rel_1to1_data: [" + col_name +
                        "] from entities: [" + std::to_string(es.size()) + "]");
  }
  m_rel_1to1_container[col_name]->set_entity(es[0]);
}

void neptune::entity::set_rel_1to1_data_null(const std::string &col_name) {
  m_rel_1to1_container[col_name]->set_null();
}

void neptune::entity::set_rel_1to1_data_undefined(const std::string &col_name) {
  m_rel_1to1_container[col_name]->set_undefined();
}

std::shared_ptr<neptune::entity> neptune::entity::get_rel_1to1_data_as_entity(
    const std::string &col_name) const {
  return m_rel_1to1_container.at(col_name)->get_entity();
}

bool neptune::entity::is_rel_1to1_data_null(const std::string &col_name) const {
  return m_rel_1to1_container.at(col_name)->is_null();
}

bool neptune::entity::is_rel_1to1_data_undefined(
    const std::string &col_name) const {
  return m_rel_1to1_container.at(col_name)->is_undefined();
}

// =============================================================================
// neptune::entity::rel_1to1_meta ==============================================
// =============================================================================

neptune::entity::rel_1to1_meta::rel_1to1_meta(std::string key_,
                                              std::string foreign_table_,
                                              std::string foreign_key_,
                                              neptune::rel_dir dir_)
    : key(std::move(key_)), foreign_table(std::move(foreign_table_)),
      foreign_key(std::move(foreign_key_)), dir(dir_) {}

const std::vector<neptune::entity::rel_1to1_meta> &
neptune::entity::iter_rel_1to1_metas() const {
  return m_rel_1to1_metas;
}

// =============================================================================
// neptune::entity::column =====================================================
// =============================================================================

neptune::entity::column::column(neptune::entity *this_ptr, std::string col_name)
    : m_col_name(std::move(col_name)),
      m_container_ref(this_ptr->m_col_container),
      m_metas_ref(this_ptr->m_col_metas) {}

std::string neptune::entity::column::get_col_name() const { return m_col_name; }

bool neptune::entity::column::is_null() const {
  return m_container_ref[m_col_name]->is_null();
}

void neptune::entity::column::set_null() {
  m_container_ref[m_col_name]->set_null();
}

bool neptune::entity::column::is_undefined() const {
  return m_container_ref[m_col_name]->is_undefined();
}

void neptune::entity::column::set_undefined() {
  m_container_ref[m_col_name]->set_undefined();
}

// =============================================================================
// neptune::entity::column_primary_generated_uint32 ============================
// =============================================================================

neptune::entity::column_primary_generated_uint32::
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name)
    : column(this_ptr, std::move(col_name)) {
  m_container_ref.emplace(m_col_name, std::make_shared<col_data_uint32>());
  m_metas_ref.emplace_back(
      m_col_name, "INT UNSIGNED AUTO_INCREMENT PRIMARY KEY", true, false);
}

std::uint32_t
neptune::entity::column_primary_generated_uint32::get_value() const {
  return std::dynamic_pointer_cast<col_data_uint32>(m_container_ref[m_col_name])
      ->get_value();
}

void neptune::entity::column_primary_generated_uint32::set_value(
    std::uint32_t value) {
  std::dynamic_pointer_cast<col_data_uint32>(m_container_ref[m_col_name])
      ->set_value(value);
}

// =============================================================================
// neptune::entity::column_varchar =============================================
// =============================================================================

neptune::entity::column_varchar::column_varchar(neptune::entity *this_ptr,
                                                std::string col_name,
                                                bool is_nullable,
                                                std::size_t max_length)
    : column(this_ptr, std::move(col_name)), m_max_length(max_length) {
  std::string datatype = "VARCHAR(" + std::to_string(max_length) + ")";
  if (!is_nullable) {
    datatype += " NOT NULL";
  }
  m_container_ref.emplace(m_col_name, std::make_shared<col_data_string>());
  m_metas_ref.emplace_back(m_col_name, datatype, false, is_nullable);
}

std::string neptune::entity::column_varchar::get_value() const {
  return std::dynamic_pointer_cast<col_data_string>(m_container_ref[m_col_name])
      ->get_value();
}

void neptune::entity::column_varchar::set_value(const std::string &value) {
  if (value.size() > m_max_length) {
    __NEPTUNE_THROW(exception_type::invalid_argument,
                    "Value is too long for column [" + m_col_name + "]");
  }
  std::dynamic_pointer_cast<col_data_string>(m_container_ref[m_col_name])
      ->set_value(value);
}

// =============================================================================
// neptune::entity::relation ===================================================
// =============================================================================

neptune::entity::relation::relation(std::string rel_key)
    : m_rel_key(std::move(rel_key)) {}

std::string neptune::entity::relation::get_rel_key() const { return m_rel_key; }

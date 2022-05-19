#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include "neptune/utils/exception.hpp"
#include "neptune/utils/typedefs.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace neptune {

class entity {
  friend class connection;
  friend class mariadb_connection;
  friend class driver;
  friend class mariadb_driver;
  friend class query_selector;

private:
  class col_data {
  protected:
    bool m_is_null, m_is_undefined;

  public:
    col_data();

    [[nodiscard]] bool is_null() const;

    void set_null();

    [[nodiscard]] bool is_undefined() const;

    void set_undefined();

    virtual void set_value_from_string(const std::string &value) = 0;

    [[nodiscard]] virtual std::string get_value_as_string() const = 0;
  };

  class col_data_uint32 : public col_data {
  private:
    std::uint32_t m_value;

  public:
    col_data_uint32();

    void set_value_from_string(const std::string &value) override;

    [[nodiscard]] std::string get_value_as_string() const override;

    [[nodiscard]] std::uint32_t get_value() const;

    void set_value(std::uint32_t value);
  };

  class col_data_int32 : public col_data {
  private:
    std::int32_t m_value;

  public:
    col_data_int32();

    void set_value_from_string(const std::string &value) override;

    [[nodiscard]] std::string get_value_as_string() const override;

    [[nodiscard]] std::int32_t get_value() const;

    void set_value(std::int32_t value);
  };

  class col_data_string : public col_data {
  private:
    std::string m_value;

  public:
    col_data_string();

    void set_value_from_string(const std::string &value) override;

    [[nodiscard]] std::string get_value_as_string() const override;

    [[nodiscard]] const std::string &get_value() const;

    void set_value(const std::string &value);
  };

  struct col_meta {
    std::string name, type;
    bool is_primary, nullable;

    col_meta(std::string name_, std::string type_, bool is_primary_,
             bool nullable_);
  };

  class rel_data {
  protected:
    bool m_is_null, m_is_undefined;

  public:
    rel_data();

    [[nodiscard]] bool is_null() const;

    void set_null();

    [[nodiscard]] bool is_undefined() const;

    void set_undefined();
  };

  class rel_data_single : public rel_data {
  private:
    std::shared_ptr<entity> m_value;
    std::string m_target_uuid;

  public:
    rel_data_single();

    void set_value(const std::shared_ptr<entity> &value);

    [[nodiscard]] std::string get_uuid() const;

    [[nodiscard]] std::shared_ptr<entity> get_value() const;

    void set_target_uuid(const std::string &target_uuid);

    [[nodiscard]] std::string get_target_uuid() const;
  };

  class rel_data_multiple : public rel_data {
  private:
    std::vector<std::shared_ptr<entity>> m_value;

  public:
    rel_data_multiple();

    void set_value(const std::vector<std::shared_ptr<entity>> &value);

    [[nodiscard]] std::vector<std::string> get_uuid() const;

    [[nodiscard]] std::vector<std::shared_ptr<entity>> get_value() const;
  };

  struct rel_meta {
    rel_meta_dir dir;
    std::string key, type, foreign_table, foreign_key;

    rel_meta(std::string key_, std::string type_, rel_meta_dir dir_,
             std::string foreign_table_, std::string foreign_key_);
  };

  void set_col_data_from_string(const std::string &col_name,
                                const std::string &value);

  void set_col_data_null(const std::string &col_name);

  void set_col_data_undefined(const std::string &col_name);

  void set_rel_target_uuid(const std::string &rel_key,
                           const std::string &target_uuid);

  void set_rel_data_null(const std::string &rel_key);

  void set_rel_data_from_entity(const std::string &rel_key,
                                const std::shared_ptr<entity> &value);

  [[nodiscard]] std::string
  get_rel_target_uuid(const std::string &rel_key) const;

  [[nodiscard]] std::string
  get_col_data_as_string(const std::string &col_name) const;

  [[nodiscard]] std::string get_rel_uuid(const std::string &rel_key) const;

  [[nodiscard]] bool is_undefined_col(const std::string &col_name) const;

  [[nodiscard]] bool is_null_col(const std::string &col_name) const;

  [[nodiscard]] bool is_undefined_rel(const std::string &rel_key) const;

  [[nodiscard]] bool is_null_rel(const std::string &rel_key) const;

  [[nodiscard]] const std::vector<col_meta> &get_col_metas() const;

  [[nodiscard]] const std::vector<rel_meta> &get_rel_metas() const;

  [[nodiscard]] std::string get_table_name() const;

  [[nodiscard]] std::string get_primary_col_name() const;

  void check_expected_rel_meta(const std::string &rel_key,
                               const std::string &type,
                               const std::string &foreign_table,
                               const std::string &foreign_key) const;

  void check_expected_rel_meta(const std::string &rel_key,
                               const std::string &type, rel_meta_dir dir,
                               const std::string &foreign_table,
                               const std::string &foreign_key) const;

  std::string m_table_name;
  std::map<std::string, std::shared_ptr<col_data>> m_col_container;
  std::vector<col_meta> m_col_metas;
  std::map<std::string, std::shared_ptr<rel_data>> m_rel_container;
  std::vector<rel_meta> m_rel_metas;

public:
  explicit entity(std::string table_name);

protected:
  class column {
  public:
    column(neptune::entity *this_ptr, std::string col_name, bool nullable,
           bool is_primary);

    column(const column &c) = delete;

    [[nodiscard]] std::string get_col_name() const;

    [[nodiscard]] bool is_undefined() const;

    void set_undefined();

    [[nodiscard]] bool is_null() const;

    void set_null();

    explicit operator bool() const;

  protected:
    /**
     * TODO remove unused m_nullable and m_is_primary
     */
    bool m_nullable;
    bool m_is_primary;
    std::string m_col_name;
    std::map<std::string, std::shared_ptr<col_data>> &m_container_ref;
    std::vector<col_meta> &m_metas_ref;

    void insert_int32_to_container_if_necessary();

    void insert_uint32_to_container_if_necessary();

    void insert_string_to_container_if_necessary();
  };

  class column_primary_generated_uint32 : public column {
  public:
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name);

    [[nodiscard]] std::uint32_t get_value() const;

    void set_value(std::uint32_t value);
  };

  class column_int32 : public column {
  public:
    column_int32(neptune::entity *this_ptr, std::string col_name,
                 bool nullable);

    [[nodiscard]] std::int32_t get_value() const;

    void set_value(std::int32_t value);
  };

  class column_varchar : public column {
  public:
    column_varchar(neptune::entity *this_ptr, std::string col_name,
                   bool nullable, std::size_t max_length);

    [[nodiscard]] std::string get_value() const;

    void set_value(const std::string &value);

  private:
    std::size_t m_max_length;
  };

  class relation {
  public:
    relation(neptune::entity *this_ptr, std::string rel_key);

    relation(const relation &c) = delete;

    [[nodiscard]] std::string get_rel_key() const;

    [[nodiscard]] bool is_undefined() const;

    void set_undefined();

    [[nodiscard]] bool is_null() const;

    void set_null();

    explicit operator bool() const;

  protected:
    std::string m_rel_key;
    std::map<std::string, std::shared_ptr<rel_data>> &m_container_ref;
    std::vector<rel_meta> &m_metas_ref;

    void insert_single_to_container_if_necessary();

    void insert_multiple_to_container_if_necessary();
  };

  template <class T> class relation_one_to_one : public relation {
  public:
    relation_one_to_one(neptune::entity *this_ptr, std::string rel_key,
                        rel_meta_dir dir, std::string foreign_table,
                        std::string foreign_key);

    [[nodiscard]] std::shared_ptr<T> get_value() const;

    void set_value(const std::shared_ptr<T> &value);
  };

private:
  column_varchar uuid{this, "__protected_uuid", false, 36};
};

} // namespace neptune

// =============================================================================
// neptune::entity::relation_one_to_one ========================================
// =============================================================================

template <class T>
neptune::entity::relation_one_to_one<T>::relation_one_to_one(
    neptune::entity *this_ptr, std::string rel_key, rel_meta_dir dir,
    std::string foreign_table, std::string foreign_key)
    : relation(this_ptr, std::move(rel_key)) {
  insert_single_to_container_if_necessary();
  m_metas_ref.emplace_back(m_rel_key, "one_to_one", dir,
                           std::move(foreign_table), std::move(foreign_key));
}

template <class T>
std::shared_ptr<T> neptune::entity::relation_one_to_one<T>::get_value() const {
  if (is_undefined()) {
    __NEPTUNE_THROW(exception_type::runtime_error,
                    "Relation [" + m_rel_key + "] is undefined");
  } else {
    return std::static_pointer_cast<rel_data_single>(
               m_container_ref.at(m_rel_key))
        ->get_value();
  }
}

template <class T>
void neptune::entity::relation_one_to_one<T>::set_value(
    const std::shared_ptr<T> &value) {
  std::static_pointer_cast<rel_data_single>(m_container_ref.at(m_rel_key))
      ->set_value(value);
}

#endif // NEPTUNEORM_ENTITY_HPP

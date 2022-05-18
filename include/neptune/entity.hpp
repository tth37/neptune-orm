#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace neptune {

class entity {
  friend class connection;
  friend class mariadb_connection;
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

  public:
    rel_data_single();

    void set_value_from_entity(const std::shared_ptr<entity> &value);

    [[nodiscard]] std::string get_value_as_uuid() const;

    [[nodiscard]] std::shared_ptr<entity> get_value() const;
  };

  class rel_data_multiple : public rel_data {
  private:
    std::vector<std::shared_ptr<entity>> m_value;

  public:
    rel_data_multiple();

    void
    set_value_from_entities(const std::vector<std::shared_ptr<entity>> &value);

    [[nodiscard]] std::vector<std::string> get_value_as_uuids() const;

    [[nodiscard]] std::vector<std::shared_ptr<entity>> get_value() const;
  };

  struct rel_meta {
    std::string key, type, dir, foreign_key;
    std::shared_ptr<entity> foreign_entity;

    rel_meta(std::string key_, std::string type_, std::string dir_,
             std::string foreign_key_, std::shared_ptr<entity> foreign_entity_);
  };

  void set_col_data_from_string(const std::string &col_name,
                                const std::string &value);

  void set_col_data_null(const std::string &col_name);

  void set_col_data_undefined(const std::string &col_name);

  [[nodiscard]] std::string
  get_col_data_as_string(const std::string &col_name) const;

  [[nodiscard]] bool is_undefined(const std::string &col_name) const;

  [[nodiscard]] bool is_null(const std::string &col_name) const;

  [[nodiscard]] const std::vector<col_meta> &get_col_metas() const;

  [[nodiscard]] std::string get_table_name() const;

  [[nodiscard]] std::string get_primary_col_name() const;

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

  private:
    std::string m_rel_key;
    std::map<std::string, std::shared_ptr<rel_data>> &m_container_ref;
    std::vector<rel_meta> &m_metas_ref;
  };

  class relation_one_to_one : public relation {
  public:



  };

private:
  column_varchar uuid{this, "__protected_uuid", false, 36};
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

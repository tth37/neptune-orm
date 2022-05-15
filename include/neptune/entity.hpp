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

  struct col_meta {
    std::string name;
    bool is_primary;

    col_meta(std::string name_, bool is_primary_);
  };

  void set_col_data_from_string(const std::string &col_name,
                                const std::string &value);

  std::string m_table_name;
  std::map<std::string, std::shared_ptr<col_data>> m_col_container;
  std::vector<col_meta> m_col_metas;

public:
  explicit entity(std::string table_name);

  // TODO set get_col_metas to private
  [[nodiscard]] const std::vector<col_meta> &get_col_metas() const;

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

  protected:
    bool m_nullable;
    bool m_is_primary;
    std::string m_col_name;
    std::map<std::string, std::shared_ptr<col_data>> &m_container_ref;
    std::vector<col_meta> &m_metas_ref;

    void insert_int32_to_container_if_necessary();

    void insert_uint32_to_container_if_necessary();
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

private:
  column_primary_generated_uint32 _protected_id;
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

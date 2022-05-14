#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace neptune {

class entity {
public:
  [[nodiscard]] const std::vector<std::string> &get_col_names() const;

private:
  class col_data {
  protected:
    bool m_is_null;

  public:
    col_data();

    [[nodiscard]] bool is_null() const;

    void set_null();

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

  std::string m_table_name;
  std::map<std::string, std::shared_ptr<col_data>> m_col_container;
  std::vector<std::string> m_col_names;

protected:
  class column {
  public:
    column(neptune::entity *this_ptr, std::string col_name, bool nullable,
           bool is_primary);

    [[nodiscard]] std::string get_col_name() const;

    [[nodiscard]] bool is_undefined() const;

    void set_undefined();

    [[nodiscard]] bool is_null() const;

    virtual void set_null() = 0;

  protected:
    bool m_nullable;
    bool m_is_primary;
    std::string m_col_name;
    std::map<std::string, std::shared_ptr<col_data>> &m_container_ref;
    std::vector<std::string> &m_names_ref;

    void insert_int32_to_container_if_necessary();

    void insert_uint32_to_container_if_necessary();
  };

  class column_primary_generated_uint32 : public column {
  public:
    column_primary_generated_uint32(neptune::entity *this_ptr,
                                    std::string col_name);

    void set_null() override;

    [[nodiscard]] std::uint32_t get_value() const;

    void set_value(std::uint32_t value);
  };

  class column_int32 : public column {
  public:
    column_int32(neptune::entity *this_ptr, std::string col_name,
                 bool nullable);

    void set_null() override;

    [[nodiscard]] std::int32_t get_value() const;

    void set_value(std::int32_t value);
  };
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include <memory>
#include <string>
#include <vector>

namespace neptune {

class entity {
public:
  explicit entity(std::string table_name);

  std::string get_table_name() const;

  std::string get_table_meta() const;

  std::string get_insert_sql() const;

  bool check_duplicated_col_names() const;

  bool check_primary_key() const;

  template <typename T> T get() const { return *(T *)this; }

private:
  std::string m_table_name;

  class column {
  public:
    explicit column(std::string col_name, bool nullable, bool is_primary_key);

    virtual std::string get_col_name() const;

    virtual std::string get_col_meta() const = 0;

    virtual std::string get_insert_value() const = 0;

    virtual void set_value_from_string(const std::string &value) = 0;

    virtual void set_null();

    bool is_primary_key() const;

  private:
    std::string m_col_name;
    bool m_is_primary_key;

  protected:
    bool m_nullable, m_is_null;
  };

public:
  std::vector<std::shared_ptr<column>> &get_cols();

protected:
  std::vector<std::shared_ptr<column>> m_cols;

  template <typename T> void define_column(const T &col);

  class column_int32 : public column {
  public:
    explicit column_int32(const std::string &col_name, bool nullable);

    std::string get_col_meta() const override;

    std::string get_insert_value() const override;

    void set_value_from_string(const std::string &value) override;

    void set_value(int32_t value);

    std::tuple<bool, int32_t> value() const;

  private:
    int32_t m_value;
  };

  class column_primary_generated_uint32 : public column {
  public:
    explicit column_primary_generated_uint32(const std::string &col_name);

    std::string get_col_meta() const override;

    std::string get_insert_value() const override;

    void set_value_from_string(const std::string &value) override;

    std::tuple<bool, uint32_t> value() const;

  private:
    uint32_t m_value;
  };
};

template <typename T> void entity::define_column(const T &col) {
  m_cols.push_back(std::make_shared<T>(col));
}

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include <memory>
#include <string>
#include <vector>

namespace neptune {

class entity {
public:
  /**
   * @brief Construct a new entity object
   * @param table_name
   */
  explicit entity(std::string table_name);

  [[nodiscard]] std::string get_table_name() const;

  [[nodiscard]] std::string get_define_table_sql_mariadb() const;

  [[nodiscard]] std::string get_insert_sql_mariadb() const;

  [[nodiscard]] std::string get_update_sql_mariadb() const;

  [[nodiscard]] std::string get_remove_sql_mariadb() const;

  [[nodiscard]] bool check_duplicated_col_names() const;

  [[nodiscard]] bool check_primary_key() const;

  [[nodiscard]] virtual std::shared_ptr<entity> duplicate() const = 0;

private:
  std::string m_table_name;

protected:
  class column {
  public:
    column(std::string col_name, bool nullable, bool is_primary);

    /**
     * @brief Get the name of the column
     * @return m_col_name
     */
    [[nodiscard]] std::string get_col_name() const;

    /**
     * @brief Get the SQL of definition of the column
     * @return SQL string
     */
    [[nodiscard]] virtual std::string
    get_define_table_col_sql_mariadb() const = 0;

    /**
     * @brief Get the SQL of the inserted value
     * @return SQL string
     */
    [[nodiscard]] virtual std::string
    get_insert_col_value_sql_mariadb() const = 0;

    /**
     * @brief Set value from SQL string
     * @param value
     */
    virtual void set_value_from_sql_mariadb(const std::string &value) = 0;

    virtual void set_null() = 0;

    [[nodiscard]] bool is_null() const;

    [[nodiscard]] bool is_undefined() const;

    [[nodiscard]] bool is_primary() const;

  protected:
    std::string m_col_name;
    bool m_nullable, m_is_primary, m_is_null, m_is_undefined;
  };

  std::vector<column *> m_cols;

public:
  /**
   * @brief Get columns pointer of the entity
   * @return m_cols
   */
  [[nodiscard]] const std::vector<column *> &get_cols() const;

protected:
  /**
   * @brief Register the column to entity
   * @param col
   */
  void define_column(column &col);

  class column_int32 : public column {
  public:
    column_int32(std::string col_name, bool nullable);

    [[nodiscard]] std::string get_define_table_col_sql_mariadb() const override;

    [[nodiscard]] std::string get_insert_col_value_sql_mariadb() const override;

    void set_value_from_sql_mariadb(const std::string &value) override;

    void set_null() override;

    [[nodiscard]] int32_t value() const;

    void set_value(int32_t value);

  private:
    int32_t m_value;
  };

  class column_primary_generated_uint32 : public column {
  public:
    explicit column_primary_generated_uint32(std::string col_name);

    [[nodiscard]] std::string get_define_table_col_sql_mariadb() const override;

    [[nodiscard]] std::string get_insert_col_value_sql_mariadb() const override;

    void set_value_from_sql_mariadb(const std::string &value) override;

    void set_null() override;

    [[nodiscard]] uint32_t value() const;

    void set_value(uint32_t value);

  private:
    uint32_t m_value;
  };
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

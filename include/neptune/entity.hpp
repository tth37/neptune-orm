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

  std::string get_table_name() const;

  std::string get_define_table_sql_mariadb() const;

  std::string get_insert_sql_mariadb() const;

  bool check_duplicated_col_names() const;

  bool check_primary_key() const;

  virtual std::shared_ptr<entity> duplicate() const = 0;

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
    std::string get_col_name() const;

    /**
     * @brief Get the SQL of definition of the column
     * @return SQL string
     */
    virtual std::string get_define_table_col_sql_mariadb() const = 0;

    /**
     * @brief Get the SQL of the inserted value
     * @return SQL string
     */
    virtual std::string get_insert_col_value_sql_mariadb() const = 0;

    /**
     * @brief Set value from SQL string
     * @param value
     */
    virtual void set_value_from_sql_mariadb(const std::string &value) = 0;

    virtual void set_null() = 0;

    bool is_null() const;

    bool is_primary() const;

  protected:
    std::string m_col_name;
    bool m_nullable, m_is_primary, m_is_null;
  };

  std::vector<column *> m_cols;

public:
  /**
   * @brief Get columns pointer of the entity
   * @return m_cols
   */
  const std::vector<column *> &get_cols() const;

protected:
  /**
   * @brief Register the column to entity
   * @param col
   */
  void define_column(column &col);

  class column_int32 : public column {
  public:
    column_int32(std::string col_name, bool nullable);

    std::string get_define_table_col_sql_mariadb() const override;

    std::string get_insert_col_value_sql_mariadb() const override;

    void set_value_from_sql_mariadb(const std::string &value) override;

    void set_null() override;

    int32_t value() const;

    void set_value(int32_t value);

  private:
    int32_t m_value;
  };

  class column_primary_generated_uint32 : public column {
  public:
    column_primary_generated_uint32(std::string col_name);

    std::string get_define_table_col_sql_mariadb() const override;

    std::string get_insert_col_value_sql_mariadb() const override;

    void set_value_from_sql_mariadb(const std::string &value) override;

    void set_null() override;

    uint32_t value() const;

    void set_value(uint32_t value);

  private:
    uint32_t m_value;
  };
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

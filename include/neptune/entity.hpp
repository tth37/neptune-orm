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

  /**
   * class col_data
   * An abstract class to store column data.
   *
   * A col_data is called "undefined" when:
   * - entity object is instantiated by user;
   * - entity object is instantiated by connection, but this column is not
   * selected;
   * - col_data is explicitly set by set_undefined()
   *
   * A col_data is called "null" when:
   * - entity object is instantiated by connection and this column is selected,
   * but there is no data in this column;
   * - col_data is explicitly set by set_null()
   *
   * class col_data is both accessible from user and from connection.
   * users can access col_data by public class column, and connection can access
   * col_data directly. by private member functions of class entity
   */
private:
  class col_data {
  public:
    col_data();
    virtual ~col_data() = default;
    [[nodiscard]] bool is_null() const;
    void set_null();
    [[nodiscard]] bool is_undefined() const;
    void set_undefined();
    virtual void set_value_from_string(const std::string &value) = 0;
    [[nodiscard]] virtual std::string get_value_as_string() const = 0;

  protected:
    bool m_is_null, m_is_undefined;
  };

private:
  class col_data_uint32 : public col_data {
  public:
    col_data_uint32();
    ~col_data_uint32() override = default;
    void set_value_from_string(const std::string &value) override;
    [[nodiscard]] std::string get_value_as_string() const override;
    [[nodiscard]] std::uint32_t get_value() const;
    void set_value(std::uint32_t value);

  private:
    std::uint32_t m_value;
  };

private:
  class col_data_string : public col_data {
  public:
    col_data_string();
    ~col_data_string() override = default;
    void set_value_from_string(const std::string &value) override;
    [[nodiscard]] std::string get_value_as_string() const override;
    [[nodiscard]] std::string get_value() const;
    void set_value(const std::string &value);

  private:
    std::string m_value;
  };

  /**
   * setters and getters for col_data
   * Called by connection only.
   */
private:
  void set_col_data_from_string(const std::string &col_name,
                                const std::string &value);
  void set_col_data_null(const std::string &col_name);
  void set_col_data_undefined(const std::string &col_name);
  [[nodiscard]] std::string
  get_col_data_as_string(const std::string &col_name) const;
  [[nodiscard]] bool is_col_data_null(const std::string &col_name) const;
  [[nodiscard]] bool is_col_data_undefined(const std::string &col_name) const;

  /**
   * struct col_meta
   * A struct to store column meta data.
   *
   * struct col_meta is not accessible from user.
   * It can be used by driver to generate define table SQL.
   * It can be used by connection to iterate over columns.
   */
private:
  struct col_meta {
    std::string name, datatype;
    bool is_primary, is_nullable;

    col_meta(std::string name_, std::string datatype_, bool is_primary_,
             bool is_nullable_);
  };

private:
  [[nodiscard]] const std::vector<col_meta> &iter_col_metas() const;

  /**
   * class rel_data
   * An abstract class to store relationship data.
   */
private:
  class rel_data {
  public:
    rel_data();
    virtual ~rel_data() = default;
    [[nodiscard]] bool is_null() const;
    void set_null();
    [[nodiscard]] bool is_undefined() const;
    void set_undefined();

  protected:
    bool m_is_null, m_is_undefined;
  };

private:
  class rel_data_1to1 : public rel_data {
  public:
    rel_data_1to1();
    ~rel_data_1to1() override = default;
    std::shared_ptr<entity> get_entity();
    void set_entity(std::shared_ptr<entity> entity);

  private:
    std::shared_ptr<entity> m_entity;
  };

  /**
   * struct rel_meta_1to1
   * A struct to store 1-to-1 relationship meta data.
   */
private:
  struct rel_1to1_meta {
    std::string key, foreign_table, foreign_key;
    rel_dir dir;

    rel_1to1_meta(std::string key_, std::string foreign_table_,
                  std::string foreign_key_, rel_dir dir_);
  };

private:
  [[nodiscard]] const std::vector<rel_1to1_meta> &iter_rel_1to1_metas() const;

private:
  std::string m_table_name;
  std::map<std::string, std::shared_ptr<col_data>> m_col_container;
  std::vector<col_meta> m_col_metas;
  std::map<std::string, std::shared_ptr<rel_data_1to1>> m_rel_1to1_container;
  std::vector<rel_1to1_meta> m_rel_1to1_metas;

  /**
   * class column
   * An abstract class to act as a user interface.
   *
   * As soon as a column is created, a meta data will be inserted into
   * m_col_metas.
   *
   * Copy constructor and assignment operator are disabled.
   */
protected:
  class column {
  public:
    column(entity *this_ptr, std::string col_name, bool is_nullable,
           bool is_primary);
    virtual ~column() = default;
    column(const column &rhs) = delete;
    column &operator=(const column &rhs) = delete;
    std::string get_col_name() const;
    bool is_null() const;
    void set_null();
    bool is_undefined() const;
    void set_undefined();

  protected:
    std::string m_col_name;
    std::map<std::string, std::shared_ptr<col_data>> &m_container_ref;
  };

protected:
  class column_primary_generated_uint32 : public column {
  public:
    column_primary_generated_uint32(entity *this_ptr, std::string col_name);
    ~column_primary_generated_uint32() override = default;
    std::uint32_t get_value() const;
    void set_value(std::uint32_t value);
  };

protected:
  class column_varchar : public column {
  public:
    column_varchar(entity *this_ptr, std::string col_name, bool is_nullable,
                   std::size_t max_length);
    ~column_varchar() override = default;
    std::string get_value() const;
    void set_value(std::string value);

  private:
    std::size_t m_max_length;
  };

public:
  entity(std::string table_name);
  entity(const entity &rhs) = delete;
};

}; // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

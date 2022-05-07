#ifndef NEPTUNEORM_ENTITY_HPP
#define NEPTUNEORM_ENTITY_HPP

#include <string>
#include <typeinfo>
#include <vector>

namespace neptune {

class entity {
public:
  explicit entity(const std::string &table_name){};

private:
  std::string m_table_name;

  class column {
  public:
    explicit column(const std::string &col_name);

    virtual std::string get_meta() const = 0;

  private:
    std::string m_col_name;
  };

  std::vector<column> m_columns;

protected:
  class column_int32 : public column {
  public:
    explicit column_int32(const std::string &col_name);

  private:
    int32_t m_value;
  };
};

class derived_entity : public entity {
public:
  derived_entity();
};

} // namespace neptune

#endif // NEPTUNEORM_ENTITY_HPP

#ifndef NEPTUNEORM_DRIVER_HPP
#define NEPTUNEORM_DRIVER_HPP

#include "neptune/connection.hpp"
#include "neptune/entity.hpp"
#include <mariadb/conncpp/Driver.hpp>
#include <memory>
#include <string>

namespace neptune {

class driver {
public:
  explicit driver(std::string db_name);

  virtual ~driver() = default;

  void register_entity(const std::shared_ptr<entity> &e);

  virtual void initialize() = 0;

  virtual std::shared_ptr<connection> create_connection() = 0;

protected:
  std::vector<std::shared_ptr<neptune::entity>> m_entities;
  std::string m_db_name;

  void check_duplicated_table_names();

  void check_duplicated_col_rel_names();

  void check_primary_key_count();

  void check_one_to_one_relations();

  static std::string parse_create_table_sql(const std::shared_ptr<entity> &e);
};

class mariadb_driver : public driver {
public:
  mariadb_driver(std::string url, std::uint32_t port, std::string user,
                 std::string password, std::string db_name);

  ~mariadb_driver() override = default;

  void initialize() override;

  std::shared_ptr<connection> create_connection() override;

private:
  std::string m_url, m_user, m_password;
  std::uint32_t m_port;
  sql::Driver *m_driver;
};

std::shared_ptr<driver>
use_mariadb_driver(std::string url, std::uint32_t port, std::string user,
                   std::string password, std::string db_name,
                   std::vector<std::shared_ptr<entity>> entities);

} // namespace neptune

#endif // NEPTUNEORM_DRIVER_HPP

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

  virtual void register_entity(std::shared_ptr<neptune::entity> entity) = 0;

  virtual void initialize() = 0;

  virtual std::shared_ptr<connection> create_connection() = 0;

protected:
  std::vector<std::shared_ptr<neptune::entity>> m_entities;

  std::string m_db_name;
};

class mariadb_driver : public driver {
public:
  mariadb_driver(std::string url, std::uint32_t port, std::string user,
                 std::string password, std::string db_name);

  ~mariadb_driver() override;

  void register_entity(std::shared_ptr<neptune::entity> entity) override;

  void initialize() override;

  std::shared_ptr<connection> create_connection() override;

private:
  std::string m_url, m_user, m_password;
  std::uint32_t m_port;
  sql::Driver *m_driver;
};

template <typename T>
driver *use_mariadb_driver(const std::string &url, std::uint32_t port,
                           const std::string &user, const std::string &password,
                           const std::string &db_name,
                           std::shared_ptr<T> entity) {
  auto driver = new neptune::mariadb_driver(url, port, user, password, db_name);
  driver->register_entity(entity);
  driver->initialize();
  return driver;
}

template <typename T, typename... Args>
driver *use_mariadb_driver(const std::string &url, std::uint32_t port,
                           const std::string &user, const std::string &password,
                           const std::string &db_name,
                           std::shared_ptr<T> entity, Args &&...args) {
  auto driver = new neptune::mariadb_driver(url, port, user, password, db_name);
  driver->register_entity(entity);
  return _use_mariadb_driver(driver, std::forward<Args>(args)...);
}

template <typename T>
driver *_use_mariadb_driver(neptune::driver *driver,
                            std::shared_ptr<T> entity) {
  driver->register_entity(entity);
  driver->initialize();
  return driver;
}

template <typename T, typename... Args>
driver *_use_mariadb_driver(neptune::driver *driver, std::shared_ptr<T> entity,
                            Args &&...args) {
  driver->register_entity(entity);
  return _use_mariadb_driver(driver, std::forward<Args>(args)...);
}

} // namespace neptune

#endif // NEPTUNEORM_DRIVER_HPP

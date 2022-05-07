#ifndef NEPTUNEORM_DRIVER_HPP
#define NEPTUNEORM_DRIVER_HPP

#include "neptune/connection.hpp"
#include <mariadb/conncpp/Driver.hpp>
#include <memory>
#include <string>

namespace neptune {

class driver {
public:
  virtual ~driver() = default;

  virtual void initialize() = 0;

  virtual std::shared_ptr<connection> create_connection() = 0;
};

class mariadb_driver : public driver {
public:
  mariadb_driver(std::string url, std::uint32_t port, std::string user,
                 std::string password);

  ~mariadb_driver() override;

  void initialize() override;

  std::shared_ptr<connection> create_connection() override;

private:
  std::string m_url, m_user, m_password;
  std::uint32_t m_port;
  sql::Driver *m_driver;
};

} // namespace neptune

#endif // NEPTUNEORM_DRIVER_HPP

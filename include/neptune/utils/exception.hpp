#ifndef NEPTUNEORM_EXCEPTION_HPP
#define NEPTUNEORM_EXCEPTION_HPP

#include "neptune/utils/logger.hpp"
#include <exception>
#include <string>

namespace neptune {

enum class exception_type {
  sql_error = 0,
  invalid_argument = 1,
  runtime_error = 2,
};

class exception : public std::exception {
public:
  exception(exception_type type, std::string message);

  ~exception() noexcept override = default;

  std::string message() const noexcept;

  exception_type type() const noexcept;

private:
  exception_type m_type;
  std::string m_message;
};

} // namespace neptune

#define __NEPTUNE_THROW(type, message)                                         \
  {                                                                            \
    __NEPTUNE_LOG(error, message);                                             \
    throw neptune::exception(type, message);                                   \
  }

#endif // NEPTUNEORM_EXCEPTION_HPP

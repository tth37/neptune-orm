#include "neptune/utils/exception.hpp"

#include <utility>

neptune::exception::exception(neptune::exception_type type, std::string message)
    : m_type(type), m_message(std::move(message)) {}

std::string neptune::exception::message() const noexcept { return m_message; }

neptune::exception_type neptune::exception::type() const noexcept {
  return m_type;
}

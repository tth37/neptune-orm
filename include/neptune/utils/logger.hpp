#ifndef NEPTUNEORM_LOGGER_HPP
#define NEPTUNEORM_LOGGER_HPP

#include <mutex>
#include <string>

namespace neptune {

class logger {
public:
  void debug(const std::string &message, const std::string &file,
             std::size_t line);

  void info(const std::string &message, const std::string &file,
            std::size_t line);

  void warn(const std::string &message, const std::string &file,
            std::size_t line);

  void error(const std::string &message, const std::string &file,
             std::size_t line);

private:
  std::mutex m_mtx;
};

void use_logger();

void debug(const std::string &message, const std::string &file,
           std::size_t line);

void info(const std::string &message, const std::string &file,
          std::size_t line);

void warn(const std::string &message, const std::string &file,
          std::size_t line);

void error(const std::string &message, const std::string &file,
           std::size_t line);

#ifdef __NEPTUNE_LOGGER_DISABLED
#define __NEPTUNE_LOG(level, msg)
#else
#define __NEPTUNE_LOG(level, msg)                                              \
  { neptune::level(msg, __FILE__, __LINE__); }
#endif

} // namespace neptune

#endif // NEPTUNEORM_LOGGER_HPP

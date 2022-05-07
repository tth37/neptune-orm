#include "neptune/utils/logger.hpp"
#include <iostream>

namespace neptune {

std::unique_ptr<logger> active_logger = nullptr;

static const char red[] = {0x1b, '[', '1', ';', '3', '1', 'm', 0};
static const char yellow[] = {0x1b, '[', '1', ';', '3', '3', 'm', 0};
static const char blue[] = {0x1b, '[', '1', ';', '3', '4', 'm', 0};
static const char green[] = {0x1b, '[', '1', ';', '3', '2', 'm', 0};
static const char normal[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};

void logger::debug(const std::string &message, const std::string &file,
                   std::size_t line) {
  std::lock_guard<std::mutex> lock(m_mtx);
  std::cout << blue << "["
            << "DEBUG"
            << "][NeptuneORM][" << file << ":" << line << "] " << normal
            << message << std::endl;
}

void logger::info(const std::string &message, const std::string &file,
                  std::size_t line) {
  std::lock_guard<std::mutex> lock(m_mtx);
  std::cout << green << "["
            << "INFO"
            << "][NeptuneORM][" << file << ":" << line << "] " << normal
            << message << std::endl;
}

void logger::warn(const std::string &message, const std::string &file,
                  std::size_t line) {
  std::lock_guard<std::mutex> lock(m_mtx);
  std::cout << yellow << "["
            << "WARN"
            << "][NeptuneORM][" << file << ":" << line << "] " << normal
            << message << std::endl;
}

void logger::error(const std::string &message, const std::string &file,
                   std::size_t line) {
  std::lock_guard<std::mutex> lock(m_mtx);
  std::cout << red << "["
            << "ERROR"
            << "][NeptuneORM][" << file << ":" << line << "] " << normal
            << message << std::endl;
}

void debug(const std::string &message, const std::string &file,
           std::size_t line) {
  if (active_logger) {
    active_logger->debug(message, file, line);
  }
}

void info(const std::string &message, const std::string &file,
          std::size_t line) {
  if (active_logger) {
    active_logger->info(message, file, line);
  }
}

void warn(const std::string &message, const std::string &file,
          std::size_t line) {
  if (active_logger) {
    active_logger->warn(message, file, line);
  }
}

void error(const std::string &message, const std::string &file,
           std::size_t line) {
  if (active_logger) {
    active_logger->error(message, file, line);
  }
}

void use_logger() {
  if (active_logger == nullptr) {
    active_logger = std::make_unique<logger>();
  }
}

} // namespace neptune

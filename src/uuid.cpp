#include "neptune/utils/uuid.hpp"
#include <random>

std::string neptune::uuid::uuid(std::size_t length) {
  std::string uuid;
  uuid.reserve(length);

  static const char uuid_charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDE"
                                     "FGHIJKLMNOPQRSTUVWXYZ-=!@#$%^&*()_+";
  static const std::size_t uuid_charset_size = sizeof(uuid_charset) - 1;

  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, uuid_charset_size - 1);

  for (std::size_t i = 0; i < length; ++i) {
    uuid += uuid_charset[dis(gen)];
  }

  return uuid;
}
#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <iostream>
#include <neptune/driver.hpp>

int main() {
  neptune::use_logger();
  try {
    auto driver =
        new neptune::mariadb_driver("127.0.0.1", 3306, "root", "root");
    driver->initialize();
    auto entity = new neptune::entity();
  } catch (neptune::exception &e) {
  }
  system("pause");
}
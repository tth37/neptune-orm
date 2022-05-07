#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <iostream>
#include <neptune/driver.hpp>

class student_entity : public neptune::entity {
public:
  student_entity() : entity("student"), score("core", true), id("id") {
    define_column(score);
    define_column(id);
  }

private:
  column_int32 score;
  column_primary_generated_uint32 id;
};

int main() {
  neptune::use_logger();
  try {
    auto driver =
        neptune::use_mariadb_driver("localhost", 3306, "root", "root", "nep",
                                    std::make_shared<student_entity>());
  } catch (neptune::exception &e) {
  }
  system("pause");
}
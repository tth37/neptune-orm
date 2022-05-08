#include "neptune/entity.hpp"
#include "neptune/utils/exception.hpp"
#include "neptune/utils/logger.hpp"
#include <iostream>
#include <neptune/driver.hpp>

class student_entity : public neptune::entity {
public:
  student_entity() : entity("student"), score("score", true), id("id") {
    define_column(score);
    define_column(id);
  }

  column_int32 score;
  column_primary_generated_uint32 id;
};

int main() {
  neptune::use_logger();
  try {
    auto driver =
        neptune::use_mariadb_driver("localhost", 3306, "root", "root", "nept",
                                    std::make_shared<student_entity>());
    auto connection = driver->create_connection();
    student_entity s;
    s.score.set_value(100);
    connection->insert(s);
    auto res = connection->select<student_entity>(connection->query());
    for (auto &r : res) {
      auto [id_is_null, id] = r.id.value();
      auto [score_is_null, score] = r.score.value();
      std::cout << "id: " << id << " score: " << score << std::endl;
    }

  } catch (neptune::exception &e) {
  }
  system("pause");
}
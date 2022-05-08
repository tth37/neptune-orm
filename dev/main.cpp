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

  std::shared_ptr<neptune::entity> duplicate() const override {
    return std::make_shared<student_entity>();
  }

  column_int32 score;
  column_primary_generated_uint32 id;
};

class user_entity : public neptune::entity {
public:
  column_primary_generated_uint32 id;
  column_int32 age;

  user_entity() : entity("user"), id("id"), age("age", true) {
    define_column(id);
    define_column(age);
  }

  std::shared_ptr<neptune::entity> duplicate() const override {
    return std::make_shared<user_entity>();
  }
};

int main() {
  neptune::use_logger();

  try {
    auto driver = neptune::use_mariadb_driver(
        "127.0.0.1", 3306, "root", "root", "zhangyuchao",
        std::make_shared<user_entity>(), std::make_shared<student_entity>());
    user_entity new_user;
    auto conn = driver->create_connection();

    new_user.age = 2;
    //  conn->insert(new_user);

    conn->update(new_user, conn->query().where({"id", "=", 3}));
    conn->remove<user_entity>(conn->query().where({"id", "=", 2}));

    auto res =
        conn->select<user_entity>(conn->query().order_by({"id", "DESC"}));
    for (int i = 0; i < res.size(); i++) {
      std::cout << res[i].id.value() << " " << res[i].age.value() << std::endl;
    }

  } catch (const neptune::exception &e) {
  }

  //  neptune::use_logger();
  //  try {
  //    auto driver =
  //        neptune::use_mariadb_driver("localhost", 3306, "root", "root",
  //        "neptu",
  //                                    std::make_shared<student_entity>());
  //    auto connection = driver->create_connection();
  //    __NEPTUNE_LOG(info, "Connection created");
  //    student_entity student;
  //    student.score.set_value(100);
  //    auto res = connection->select<student_entity>(connection->query());
  //    for (auto &r : res) {
  //      __NEPTUNE_LOG(debug, "id: " + std::to_string(r.id.value()));
  //      __NEPTUNE_LOG(debug, "score: " + std::to_string(r.score.value()));
  //    }
  //  } catch (neptune::exception &e) {
  //  }
  system("pause");
}
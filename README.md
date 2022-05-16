# NeptuneORM - An ORM library for modern C++

## Quick Start

To initialize a mariadb driver and a user entity:

```c++
#include <iostream>
#include <neptune/neptune.hpp>

using namespace neptune;

class user_entity : public entity {
public:
  column_primary_generated_uint32 id;
  column_int32 age;

  // set table_name to "user"
  // set column "id" to primary generated column
  // set column "age" to nullable int column
  user_entity() : entity("user"), id("id"), age("age", true) {
    define_column(id);
    define_column(age);
  }

  // implement duplicate function
  std::shared_ptr<entity> duplicate() const override {
    return std::make_shared<user_entity>();
  }
};

int main() {
  use_logger();

  // initialize mariadb_driver and register user_entity
  auto driver =
      use_mariadb_driver("127.0.0.1", 3306, "root", "root", "neptune_ex",
                         std::make_shared<user_entity>());
  auto conn = driver->create_connection();

  // insert new_user to database
  user_entity new_user;
  new_user.age = 10;
  conn->insert(new_user);

  // update new_user's age to 20
  new_user.age = 20;
  conn->update(new_user, conn->query().where("id", "=", 1));

  // select the user with "id = 1"
  auto res = conn->select<user_entity>(conn->query().where("id", "=", 1));
  for (auto &user : res) {
    std::cout << "id: " << user.id.value() << std::endl;
    std::cout << "age: " << user.age.value() << std::endl;
  }

  // delete the user with "id = 1"
  conn->remove<user_entity>(conn->query().where("id", "=", 1));

  return 0;
}


```
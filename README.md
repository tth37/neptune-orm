# NeptuneORM - An easy to use ORM library for C++

## Quick Start

To initialize a mariadb driver and a user entity:

```c++
#include <iostream>
#include <neptune/neptune.hpp>

using namespace neptune;

class user_entity : public entity {
public:
  // set table_name to "user"
  user_entity() : entity("user") {}
  // set column "id" as primary generated column
  column_primary_generated_uint32 id{this, "id"};
  // set column "age" as int32 column
  column_int32 age{this, "age", true};
  // set column "name" as varchar column with length 32
  column_varchar name{this, "name", false, 32};
};

int main() {
  // use neptune default logger
  use_logger();

  // initialize mariadb_driver and register user_entity
  auto driver = use_mariadb_driver("127.0.0.1", 3306, "root", "root", "demo",
                                   {std::make_shared<user_entity>()});

  // create connection to database
  auto conn = driver->create_connection();

  // insert new_user to database
  auto new_user = std::make_shared<user_entity>();
  new_user->age.set_value(28);
  new_user->name.set_value("George");
  auto inserted_user = conn->insert(new_user);

  // select one user where id = 1
  auto user = conn->select_one<user_entity>(conn->query().where("id", "=", 1));
  if (user != nullptr) {
    // update user's name to "James"
    user->name.set_value("James");
    // set user's age to NULL
    user->age.set_null();
    conn->update(user);
  }

  // select all users
  auto users = conn->select<user_entity>(conn->query());
  for (const auto &cur_user : users) {
    if (cur_user->id)
      std::cout << "user id: " << cur_user->id.get_value() << std::endl;
    if (cur_user->age)
      std::cout << "user age: " << cur_user->age.get_value() << std::endl;
    if (cur_user->name)
      std::cout << "user name: " << cur_user->name.get_value() << std::endl;
    std::cout << std::endl;
  }

  system("pause");
  return 0;
}
```
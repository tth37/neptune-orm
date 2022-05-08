# NeptuneORM - An easy to use ORM library for C++

## Quick Start

To initialize a mariadb driver and a user entity:

```c++
#include <neptune/neptune.hpp>
using namespace neptune;

class user_entity : public entity {
public:
    column_primary_generated_uint32 id;
    column_int32 age;
    column_varchar name;
    
    user_entity() : entity("user"), id("id"), age("age", true), name("name", true) {
        define_column(id);
        define_column(age);
        define_column(name);
    }
    
    std::shared_ptr<entity> duplicate() const override {
        return std::make_shared<user_entity>();
    }
};

int main() {
    neptune::use_logger();
    auto driver = neptune::use_mariadb_driver(
        "127.0.0.1", 3306, "root", "root", "neptune",
        std::make_shared<user_entity>());
    auto conn = driver->create_connection();
    
    user_entity new_user;
    new_user.age = 10;
    new_user.name = "Kitty";
    conn->insert(new_user);
    
    new_user.age = 20;
    conn->update(new_user);
    
    auto res = conn->select_one<user_entity>(conn->query().where("id", "=", 1));
    std::cout << res->name.value() << std::endl;
    std::cout << res->age.value() << std::endl;
    
    return 0;
}


```
# NeptuneORM - An easy to use ORM library for C++

## Quick Start

To initialize a mariadb driver and a user entity:

```c++
#include <iostream>
#include <neptune/neptune.hpp>
#include <vector>
using namespace std;
using namespace neptune;

class student_entity : public entity {
public:
  column_primary_generated_uint32 id{this, "id"};
  column_int32 age{this, "age", false};
  column_varchar name{this, "name", false, 32};
  student_entity() : entity("student") {}
};

int main() {
  use_logger();

  try {
    auto driver = make_shared<mariadb_driver>("127.0.0.1", 3306, "root", "root",
                                              "neptune_ex8");
    driver->register_entity(make_shared<student_entity>());
    driver->initialize();
    auto conn = driver->create_connection();

    //    auto new_student = make_shared<student_entity>();
    //    new_student->age.set_value(28);
    //    new_student->name.set_value("George");
    //
    //    auto inserted_student = conn->insert(new_student);
    //

    //    cout << "inserted student id: " << inserted_student->id.get_value() <<
    //    endl; cout << "inserted student age: " <<
    //    inserted_student->age.get_value()
    //         << endl;
    //    cout << "inserted student name: " <<
    //    inserted_student->name.get_value()
    //         << endl;

    auto students = conn->select<student_entity>(
        query().select("age").select("name").where("id", ">", 1));
    for (auto student : students) {
      if (student->id)
        cout << "student id: " << student->id.get_value() << endl;
      if (student->age)
        cout << "student age: " << student->age.get_value() << endl;
      if (student->name)
        cout << "student name: " << student->name.get_value() << endl;
      cout << endl;
    }

  } catch (neptune::exception &e) {
  }

  system("pause");
  return 0;
}
```
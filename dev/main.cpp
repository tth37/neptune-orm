//#include <iostream>
//#include <neptune/neptune.hpp>
//#include <vector>
// using namespace std;
// using namespace neptune;
//
// class student_entity : public entity {
// public:
//  column_primary_generated_uint32 id{this, "id"};
//  column_int32 age{this, "age", false};
//  column_varchar name{this, "name", false, 32};
//  student_entity() : entity("student") {}
//};
//
// int main() {
//  use_logger();
//
//  try {
//    auto driver = make_shared<mariadb_driver>("127.0.0.1", 3306, "root",
//    "root",
//                                              "neptune_ex8");
//    driver->register_entity(make_shared<student_entity>());
//    driver->initialize();
//    auto conn = driver->create_connection();
//
//    //    auto new_student = make_shared<student_entity>();
//    //    new_student->age.set_value(28);
//    //    new_student->name.set_value("George");
//    //
//    //    auto inserted_student = conn->insert(new_student);
//    //
//
//    //    cout << "inserted student id: " << inserted_student->id.get_value()
//    <<
//    //    endl; cout << "inserted student age: " <<
//    //    inserted_student->age.get_value()
//    //         << endl;
//    //    cout << "inserted student name: " <<
//    //    inserted_student->name.get_value()
//    //         << endl;
//
//    auto student =
//        conn->select_one<student_entity>(query().where("id", "=", 3));
//    //    student->name.set_value("Shiwt fufufu");
//    //    conn->update(student);
//    conn->remove(student);
//
//    auto students = conn->select<student_entity>(query());
//    for (auto student : students) {
//      if (student->id)
//        cout << "student id: " << student->id.get_value() << endl;
//      if (student->age)
//        cout << "student age: " << student->age.get_value() << endl;
//      if (student->name)
//        cout << "student name: " << student->name.get_value() << endl;
//      cout << endl;
//    }
//
//  } catch (neptune::exception &e) {
//  }
//
//  system("pause");
//  return 0;
//}

//#include <iostream>
//#include <neptune/neptune.hpp>
//
// using namespace neptune;
//
// class teacher_entity : public entity {
// public:
//  teacher_entity() : entity("teacher") {}
//};
//
// class user_entity : public entity {
// public:
//  // set table_name to "user"
//  user_entity() : entity("user") {}
//  // set column "id" as primary generated column
//  column_primary_generated_uint32 id{this, "id"};
//  // set column "age" as int32 column
//  column_int32 age{this, "age", true};
//  // set column "name" as varchar column with length 32
//  column_varchar name{this, "name", false, 32};
//
//  relation_one_to_one<teacher_entity> teacher{this, "teacher", left,
//  "student"};
//};
//
// int main() {
//  // use neptune default logger
//  use_logger();
//
//  // initialize mariadb_driver and register user_entity
//  auto driver = use_mariadb_driver("127.0.0.1", 3306, "root", "root", "demo",
//                                   {std::make_shared<user_entity>()});
//
//  // create connection to database
//  auto conn = driver->create_connection();
//
//  auto teacher = std::make_shared<teacher_entity>();
//
//  // insert new_user to database
//  auto new_user = std::make_shared<user_entity>();
//  new_user->age.set_value(28);
//  new_user->name.set_value("George");
//  new_user->teacher.set_value(teacher);
//  //  auto inserted_user = conn->insert(new_user);
//  //
//  //  // select one user where id = 1
//  //  auto user = conn->select_one<user_entity>(conn->query().where("id", "=",
//  //  1)); if (user != nullptr) {
//  //    // update user's name to "James"
//  //    user->name.set_value("James");
//  //    // set user's age to NULL
//  //    user->age.set_null();
//  //    conn->update(user);
//  //  }
//  //
//  //  // select all users
//  //  auto users = conn->select<user_entity>(conn->query());
//  //  for (const auto &cur_user : users) {
//  //    if (cur_user->id)
//  //      std::cout << "user id: " << cur_user->id.get_value() << std::endl;
//  //    if (cur_user->age)
//  //      std::cout << "user age: " << cur_user->age.get_value() << std::endl;
//  //    if (cur_user->name)
//  //      std::cout << "user name: " << cur_user->name.get_value() <<
//  std::endl;
//  //    std::cout << std::endl;
//  //  }
//
//  system("pause");
//  return 0;
//}

#include <iostream>
#include <neptune/neptune.hpp>

using namespace neptune;

class teacher_entity;
class student_entity;

class teacher_entity : public entity {
public:
  teacher_entity() : entity("teacher") {}
  column_primary_generated_uint32 id{this, "id"};
  column_varchar name{this, "name", false, 32};
  relation_1to1<student_entity> student{this, "student", left, "student",
                                        "teacher"};
};

class student_entity : public entity {
public:
  student_entity() : entity("student") {}
  column_primary_generated_uint32 id{this, "id"};
  column_varchar name{this, "name", false, 32};
  relation_1to1<teacher_entity> teacher{this, "teacher", right, "teacher",
                                        "student"};
};

int main() {
  // use neptune default logger
  use_logger();

  query_selector selector;
  selector.where(query_selector::or_(
      query_selector::or_({{"id", "=", 1}}, {{"id", "=", 3}}),
      {{"id", "=", 2}}));

  try {
    // initialize mariadb_driver and register user_entity
    auto driver = use_mariadb_driver("127.0.0.1", 3306, "root", "root",
                                     "dessdddsddddfmdo",
                                     {std::make_shared<student_entity>(),
                                      std::make_shared<teacher_entity>()});
    auto conn = driver->create_connection();
    auto teacher = std::make_shared<teacher_entity>();
    auto student = std::make_shared<student_entity>();
    teacher->name.set_value("teacher");
    auto inserted_teacher = conn->insert(teacher);
    auto result = conn->select<teacher_entity>(
        query_selector::query().relation("student"));

  } catch (neptune::exception &e) {
    std::cout << e.message() << std::endl;
  }

  system("pause");
  return 0;
}
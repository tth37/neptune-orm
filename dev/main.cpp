#include <iostream>
#include <neptune/neptune.hpp>
#include <vector>
using namespace std;
using namespace neptune;

class student_entity : public entity {
public:
  column_int32 age;
  student_entity() : entity("student"), age(this, "age", false) {}
};

int main() {
  use_logger();

  student_entity student;
  auto metas = student.get_col_metas();
  for (const auto &meta : metas) {
    cout << meta.name << endl;
  }

  student_entity student2(student);
  metas = student.get_col_metas();
  for (const auto &meta : metas) {
    cout << meta.name << endl;
  }

  system("pause");
  return 0;
}
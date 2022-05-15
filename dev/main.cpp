#include <iostream>
#include <neptune/neptune.hpp>
#include <vector>
using namespace std;
using namespace neptune;

class student_entity : public entity {
public:
  column_int32 age{this, "age", false};
  student_entity() : entity("student") {}
};

int main() {
  use_logger();

  auto student = make_shared<student_entity>();
  student->age.set_value(20);

  system("pause");
  return 0;
}
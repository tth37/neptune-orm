#include <iostream>
#include <neptune/neptune.hpp>
#include <vector>
using namespace std;

class student_entity : public neptune::entity {
  column id;
  student_entity() : id(this, "id", false, false) {}
};

int main() {

  system("pause");
  return 0;
}
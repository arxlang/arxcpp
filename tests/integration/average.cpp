#include <iostream>

extern "C" {
float average(float, float);
}

int main() {
  float avg_3_4 = average(3.0, 4.0);
  std::cout << "average of 3.0 and 4.0: " << avg_3_4 << std::endl;
}

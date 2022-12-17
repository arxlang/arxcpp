#include <iostream>

extern "C" {
double average(double, double);
}

int main() {
  double avg_3_4 = average(3.0, 4.0);
  std::cout << "average of 3.0 and 4.0: " << avg_3_4 << std::endl;
}

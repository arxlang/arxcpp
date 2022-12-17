#include <iostream>

extern "C" {
double sum(double, double);
}

int main() {
  double sum_3_4 = sum(3.0, 4.0);
  std::cout << "sum of 3.0 and 4.0: " << sum_3_4 << std::endl;
}

#include <iostream>

extern "C" {
double arithmetic_progression_10(double, double);
}

int main() {
  double result = arithmetic_progression_10(10);
  std::cout << "arithmetic progression_from 1 to 10 with constant 10 is: "
            << result << std::endl;
}

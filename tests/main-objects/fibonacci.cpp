#include <iostream>

extern "C" {
double fib(double);
}

int main() {
  double fib_10 = fib(10.);
  std::cout << "fib(40.): " << fib_10 << std::endl;
}

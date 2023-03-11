#include <arrow-glib/arrow-glib.h>
#include <iostream>

extern "C" {
GArrowFloatScalar fib(double);
}

int main() {
  GArrowFloatScalar fib_10 = fib(10.);
  std::cout << "fib(40.): " << fib_10 << std::endl;
}

#include <arrow-glib/arrow-glib.h>
#include <iostream>

extern "C" {
float fib(float);
}

int main() {
  float fib_10 = fib(10.);
  std::cout << "fib(10.): " << fib_10 << std::endl;
}

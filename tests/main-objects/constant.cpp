#include <iostream>

extern "C" {
double get_constant(double);
}

int main() {
  double value = get_constant(3.0);
  printf("get_constant(3.0): %f\n", value);

  return 0;
}

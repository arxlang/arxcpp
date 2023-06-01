#include <iostream>

extern "C" {
float get_constant(float);
}

int main() {
  float value = get_constant(3.0);
  printf("get_constant(3.0): %f\n", value);

  return 0;
}

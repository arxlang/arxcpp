#include <arrow-glib/arrow-glib.h>
#include <iostream>

extern "C" {
GArrowFloatScalar sum(double, double);
}

int main() {
  GArrowFloatScalar sum_3_4 = sum(3.0, 4.0);
  std::cout << "sum of 3.0 and 4.0: " << sum_3_4 << std::endl;
}

#include <arrow-glib/arrow-glib.h>
#include <iostream>

extern "C" {
float print_star(float);
}

extern "C" auto putchard(float X) -> float {
  fputc(static_cast<char>(X), stderr);
  return 0;
}

int main() {
  std::cout << "print 10 *:" << std::endl;
  print_star(10);
  std::cout << std::endl;
}

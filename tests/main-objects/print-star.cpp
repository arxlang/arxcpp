#include <iostream>

extern "C" {
double print_star(double);
}

extern "C" auto putchard(double X) -> double {
  fputc((char) X, stderr);
  return 0;
}

int main() {
  std::cout << "print 10 *:" << std::endl;
  print_star(10);
  std::cout << std::endl;
}

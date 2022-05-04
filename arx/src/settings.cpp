#include <map>

#include "settings.h"

// BinopPrecedence from "parser.h"
extern std::map<char, int> BinopPrecedence;

void load_settings() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['='] = 2;
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
}

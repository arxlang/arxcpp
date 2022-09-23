#include <map>

#include "settings.h"

/**
 * BinopPrecedence from "parser.h"
 */
extern std::map<char, int> BinopPrecedence;

/**
 * @brief Install standard binary operators.
 * 1 is lowest precedence. 
 */
void load_settings() {
  BinopPrecedence['='] = 2;
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
}

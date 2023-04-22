#pragma once

#include <string>

/**
 * @brief Tokenize the known variables by the lexer
 *
 * The lexer returns tokens [0-255] if it is an unknown character, otherwise
 * one of these for known things.
 */
enum Token {
  tok_eof = -1,

  // commands
  tok_function = -2,
  tok_extern = -3,
  tok_return = -4,

  // primary
  tok_identifier = -10,
  tok_float_literal = -11,

  // control
  tok_if = -20,
  tok_then = -21,
  tok_else = -22,
  tok_for = -23,
  tok_in = -24,

  // operators
  tok_binary = -30,
  tok_unary = -31,

  // var definition
  tok_var = -40,
  tok_const = -41,
  tok_arrow_right = -42,

  tok_not_initialized = -9999,
  tok_expression = -10000  // generic used just for error message
};

struct SourceLocation {
  int line = 1;
  int col = 0;
};

class Lexer {
 public:
  static SourceLocation cur_loc;
  static std::string identifier_str;  // Filled in if tok_identifier
  static float num_float;             // Filled in if tok_float_literal
  static int cur_tok;
  static SourceLocation lex_loc;

  static std::string get_tok_name(int);
  static std::string get_tok_name_display(int);
  static int gettok();
  static int advance();
  static int get_next_token();
};

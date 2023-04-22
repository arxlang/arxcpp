#include "lexer.h"  // for Lexer, SourceLocation, tok_binary, tok_else, tok_eof
#include <cctype>   // for isdigit, isalnum, isalpha, isspace
#include <cstdio>   // for EOF
#include <cstdlib>  // for strtod
#include <string>   // for operator==, allocator, string, basic_string
#include "io.h"     // for get_char

SourceLocation Lexer::cur_loc;
// Filled in if tok_identifier
std::string Lexer::identifier_str = "<NOT DEFINED>";
// Filled in if tok_float_literal
float Lexer::num_float;
SourceLocation Lexer::lex_loc;
int Lexer::cur_tok = tok_not_initialized;

/**
 * @brief Get the Token name.
 * @param tok The token
 * @return Token name
 *
 */
auto Lexer::get_tok_name(int tok) -> std::string {
  switch (tok) {
    case tok_eof:
      return "eof";
    case tok_function:
      return "function";
    case tok_return:
      return "return";
    case tok_extern:
      return "extern";
    case tok_identifier:
      return "identifier";
    case tok_float_literal:
      return "float";
    case tok_if:
      return "if";
    case tok_then:
      return "then";
    case tok_else:
      return "else";
    case tok_for:
      return "for";
    case tok_in:
      return "in";
    case tok_binary:
      return "binary";
    case tok_unary:
      return "unary";
    case tok_var:
      return "var";
    case tok_const:
      return "const";
    case tok_arrow_right:
      return "->";
  }
  return std::string(1, static_cast<char>(Tok));
}

/**
 * @brief Get the Token name to be used in a message.
 * @param Tok The token
 * @return Token name
 *
 */
auto Lexer::get_tok_name_display(int Tok) -> std::string {
  switch (Tok) {
    case tok_eof:
      return "<eof>";
    case tok_function:
      return "<function>";
    case tok_return:
      return "<return>";
    case tok_extern:
      return "<extern>";
    case tok_identifier:
      return "<identifier>";
    case tok_float_literal:
      return "<float>";
    case tok_if:
      return "<if>";
    case tok_then:
      return "<then>";
    case tok_else:
      return "<else>";
    case tok_for:
      return "<for>";
    case tok_in:
      return "<in>";
    case tok_binary:
      return "<binary>";
    case tok_unary:
      return "<unary>";
    case tok_var:
      return "<var>";
    case tok_const:
      return "<const>";
    case tok_arrow_right:
      return "->";
    case tok_expression:
      // just used for error message
      return "<expression>";
  }
  return std::string(1, static_cast<char>(tok));
}

/**
 * @brief Check if given character is a valid first identifier character.
 * @param c A single character for checking the token
 * @return true if the token is valid, otherwise, false.
 *
 */
static auto is_identifier_first_char(char c) -> bool {
  return isalpha(c) || c == '_';
}

/**
 * @brief Check if the given character is a valid identifier char.
 * @param c Given character from a token.
 * @return true if the is a valid character, otherwise, false.
 *
 */
static auto is_identifier_char(char c) -> bool {
  return isalnum(c) || c == '_';
}

/**
 * @brief advance the token from the buffer.
 * @return Token in integer form.
 *
 */
auto Lexer::advance() -> int {
  int last_char = get_char();

  if (last_char == '\n' || last_char == '\r') {
    Lexer::lex_loc.line++;
    Lexer::lex_loc.col = 0;
  } else {
    Lexer::lex_loc.col++;
  }
  return last_char;
}

/**
 * @brief Get the next token.
 * @return Return the next token from standard input.
 *
 */
auto Lexer::gettok() -> int {
  static char last_char = ' ';

  // Skip any whitespace.
  while (isspace(last_char)) {
    last_char = static_cast<char>(Lexer::advance());
  }

  Lexer::cur_loc = Lexer::lex_loc;

  if (is_identifier_first_char(last_char)) {
    Lexer::identifier_str = static_cast<char>(last_char);
    while (
      is_identifier_char((last_char = static_cast<char>(Lexer::advance())))) {
      Lexer::identifier_str += last_char;
    }

    if (Lexer::identifier_str == "fn") {
      return tok_function;
    }
    if (Lexer::identifier_str == "return") {
      return tok_return;
    }
    if (Lexer::identifier_str == "extern") {
      return tok_extern;
    }
    if (Lexer::identifier_str == "if") {
      return tok_if;
    }
    if (Lexer::identifier_str == "else") {
      return tok_else;
    }
    if (Lexer::identifier_str == "for") {
      return tok_for;
    }
    if (Lexer::identifier_str == "in") {
      return tok_in;
    }
    if (Lexer::identifier_str == "binary") {
      return tok_binary;
    }
    if (Lexer::identifier_str == "unary") {
      return tok_unary;
    }
    if (Lexer::identifier_str == "var") {
      return tok_var;
    }
    return tok_identifier;
  }

  // Number: [0-9.]+
  if (isdigit(last_char) || last_char == '.') {
    std::string num_str;
    do {
      num_str += static_cast<char>(last_char);
      last_char = static_cast<char>(Lexer::advance());
    } while (isdigit(last_char) || last_char == '.');

    Lexer::num_float = strtod(num_str.c_str(), nullptr);
    return tok_float_literal;
  }

  // Comment until end of line.
  if (last_char == '#') {
    do {
      last_char = static_cast<char>(Lexer::advance());
    } while (last_char != EOF && last_char != '\n' && last_char != '\r');

    if (last_char != EOF) {
      return Lexer::gettok();
    }
  }

  // Check for end of file.  Don't eat the EOF.
  if (last_char == EOF) {
    return tok_eof;
  }

  // Otherwise, just return the character as its ascii value.
  int this_char = last_char;
  last_char = static_cast<char>(Lexer::advance());

  if (this_char == (int) '-' && last_char == (int) '>') {
    last_char = static_cast<char>(Lexer::advance());
    return tok_arrow_right;
  }

  return this_char;
}

/**
 * @brief Provide a simple token buffer.
 * @return
 * cur_tok is the current token the parser is looking at.
 * get_next_token reads another token from the lexer and updates
 * cur_tok with its results.
 */
auto Lexer::get_next_token() -> int {
  return Lexer::cur_tok = Lexer::gettok();
}

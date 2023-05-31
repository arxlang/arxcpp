#include "lexer.h"  // for Lexer, SourceLocation, tok_binary, tok_else, tok_eof
#include <cctype>   // for isdigit, isalnum, isalpha, isspace
#include <cstdio>   // for EOF
#include <cstdlib>  // for strtod
#include <string>   // for operator==, allocator, string, basic_string
#include "io.h"     // for get_char

SourceLocation Lexer::cur_loc;
std::string Lexer::identifier_str =
  "<NOT DEFINED>";        // Filled in if tok_identifier
double Lexer::num_float;  // Filled in if tok_float_literal
SourceLocation Lexer::lex_loc;
int Lexer::cur_tok = tok_not_initialized;

/**
 * @brief Get the Token name.
 * @param Tok The token
 * @return Token name
 *
 */
auto Lexer::get_tok_name(int Tok) -> std::string {
  switch (Tok) {
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
  }
  return std::string(1, static_cast<char>(Tok));
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
  int LastChar = get_char();

  if (LastChar == '\n' || LastChar == '\r') {
    Lexer::lex_loc.line++;
    Lexer::lex_loc.Col = 0;
  } else {
    Lexer::lex_loc.Col++;
  }
  return LastChar;
}

/**
 * @brief Get the next token.
 * @return Return the next token from standard input.
 *
 */
auto Lexer::gettok() -> int {
  static char LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    LastChar = static_cast<char>(Lexer::advance());
  }

  Lexer::cur_loc = Lexer::lex_loc;

  if (is_identifier_first_char(LastChar)) {
    Lexer::identifier_str = static_cast<char>(LastChar);
    while (
      is_identifier_char((LastChar = static_cast<char>(Lexer::advance())))) {
      Lexer::identifier_str += LastChar;
    }

    if (Lexer::identifier_str == "function") {
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
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += static_cast<char>(LastChar);
      LastChar = static_cast<char>(Lexer::advance());
    } while (isdigit(LastChar) || LastChar == '.');

    Lexer::num_float = strtod(NumStr.c_str(), nullptr);
    return tok_float_literal;
  }

  // Comment until end of line.
  if (LastChar == '#') {
    do {
      LastChar = static_cast<char>(Lexer::advance());
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF) {
      return Lexer::gettok();
    }
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF) {
    return tok_eof;
  }

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = static_cast<char>(Lexer::advance());
  return ThisChar;
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

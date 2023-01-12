#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iosfwd>  // for stringstream
#include <string>

#include "io.h"
#include "lexer.h"

SourceLocation Lexer::CurLoc;
std::string Lexer::IdentifierStr =
  "<NOT DEFINED>";     // Filled in if tok_identifier
double Lexer::NumVal;  // Filled in if tok_number
SourceLocation Lexer::LexLoc;
int Lexer::CurTok = tok_not_initialized;

/**
 * @brief Get the Token name.
 * @param Tok The token
 * @return Token name
 *
 */
auto Lexer::getTokName(int Tok) -> std::string {
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
    case tok_number:
      return "number";
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
    Lexer::LexLoc.Line++;
    Lexer::LexLoc.Col = 0;
  } else {
    Lexer::LexLoc.Col++;
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

  Lexer::CurLoc = Lexer::LexLoc;

  if (is_identifier_first_char(LastChar)) {
    Lexer::IdentifierStr = static_cast<char>(LastChar);
    while (
      is_identifier_char((LastChar = static_cast<char>(Lexer::advance())))) {
      Lexer::IdentifierStr += LastChar;
    }

    if (Lexer::IdentifierStr == "function") {
      return tok_function;
    }
    if (Lexer::IdentifierStr == "return") {
      return tok_return;
    }
    if (Lexer::IdentifierStr == "extern") {
      return tok_extern;
    }
    if (Lexer::IdentifierStr == "if") {
      return tok_if;
    }
    if (Lexer::IdentifierStr == "else") {
      return tok_else;
    }
    if (Lexer::IdentifierStr == "for") {
      return tok_for;
    }
    if (Lexer::IdentifierStr == "in") {
      return tok_in;
    }
    if (Lexer::IdentifierStr == "binary") {
      return tok_binary;
    }
    if (Lexer::IdentifierStr == "unary") {
      return tok_unary;
    }
    if (Lexer::IdentifierStr == "var") {
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

    Lexer::NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
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
 * CurTok is the current token the parser is looking at.
 * getNextToken reads another token from the lexer and updates
 * CurTok with its results.
 */
auto Lexer::getNextToken() -> int {
  return Lexer::CurTok = Lexer::gettok();
}

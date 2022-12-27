#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iosfwd>  // for stringstream
#include <string>

#include "io.h"
#include "lexer.h"

std::stringstream buffer;

SourceLocation Lexer::CurLoc;
std::string Lexer::IdentifierStr;  // Filled in if tok_identifier
double Lexer::NumVal;              // Filled in if tok_number
int Lexer::CurTok;
SourceLocation Lexer::LexLoc;

/**
 * @brief
 * @param Tok
 * @return
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
  return std::string(1, (char) Tok);
}

/**
 * @brief
 * @param c
 * @return
 *
 */
static auto is_identifier_first_char(char c) -> bool {
  return isalpha(c) || c == '_';
}

/**
 * @brief
 * @param c
 * @return
 *
 */
static auto is_identifier_char(char c) -> bool {
  return isalnum(c) || c == '_';
}

/**
 * @brief
 * @return
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
 * @brief
 * @return Return the next token from standard input.
 *
 */
auto Lexer::gettok() -> int {
  char LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    LastChar = (char) Lexer::advance();
  }

  Lexer::CurLoc = Lexer::LexLoc;

  if (is_identifier_first_char(LastChar)) {
    Lexer::IdentifierStr = (char) LastChar;
    while (is_identifier_char((LastChar = (char) Lexer::advance()))) {
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
      NumStr += (char) LastChar;
      LastChar = (char) Lexer::advance();
    } while (isdigit(LastChar) || LastChar == '.');

    Lexer::NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  // Comment until end of line.
  if (LastChar == '#') {
    do {
      LastChar = advance();
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
  LastChar = Lexer::advance();
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

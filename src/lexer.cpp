#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iosfwd>  // for stringstream
#include <string>

#include "io.h"
#include "lexer.h"

std::stringstream buffer;

SourceLocation CurLoc;
SourceLocation LexLoc;
std::string IdentifierStr;
double NumVal;

/**
 * @brief
 * @param Tok
 * @return
 *
 */
auto getTokName(int Tok) -> std::string {
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
auto advance() -> int {
  int LastChar = get_char();

  if (LastChar == '\n' || LastChar == '\r') {
    LexLoc.Line++;
    LexLoc.Col = 0;
  } else {
    LexLoc.Col++;
  }
  return LastChar;
}

/**
 * @brief
 * @return Return the next token from standard input.
 *
 */
auto gettok() -> int {
  static char LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    LastChar = (char) advance();
  }

  CurLoc = LexLoc;

  if (is_identifier_first_char(LastChar)) {
    IdentifierStr = (char) LastChar;
    while (is_identifier_char((LastChar = (char) advance()))) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "function") {
      return tok_function;
    }
    if (IdentifierStr == "return") {
      return tok_return;
    }
    if (IdentifierStr == "extern") {
      return tok_extern;
    }
    if (IdentifierStr == "if") {
      return tok_if;
    }
    if (IdentifierStr == "else") {
      return tok_else;
    }
    if (IdentifierStr == "for") {
      return tok_for;
    }
    if (IdentifierStr == "in") {
      return tok_in;
    }
    if (IdentifierStr == "binary") {
      return tok_binary;
    }
    if (IdentifierStr == "unary") {
      return tok_unary;
    }
    if (IdentifierStr == "var") {
      return tok_var;
    }
    return tok_identifier;
  }

  // Number: [0-9.]+
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += (char) LastChar;
      LastChar = (char) advance();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  // Comment until end of line.
  if (LastChar == '#') {
    do
      LastChar = advance();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF) {
    return tok_eof;
  }

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = advance();
  return ThisChar;
}

/**
 * @brief Provide a simple token buffer.
 * @return

 * CurTok is the current token the parser is looking at.
 * getNextToken reads another token from the lexer and updates
 * CurTok with its results.
 */
int CurTok;

auto getNextToken() -> int {
  return CurTok = gettok();
}

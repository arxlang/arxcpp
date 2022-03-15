#include "lexer.h"
#include <string>

std::string getTokName(int Tok) {
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
  return std::string(1, (char)Tok);
}

/* define IOSource */
char* IOSource::content = nullptr;

char IOSource::getchar() {
  if (IOSource::content) {
    return *IOSource::content++;
  }
  return getchar();
}

static bool is_identifier_first_char(char c) {
  return isalpha(c) || c == '_';
}

static bool is_identifier_char(char c) {
  return isalnum(c) || c == '_';
}

int advance() {
  int LastChar = IOSource::getchar();

  if (LastChar == '\n' || LastChar == '\r') {
    LexLoc.Line++;
    LexLoc.Col = 0;
  } else
    LexLoc.Col++;
  return LastChar;
}

/// gettok - Return the next token from standard input.
int gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = advance();

  CurLoc = LexLoc;

  if (is_identifier_first_char(LastChar)) {
    IdentifierStr = LastChar;
    while (is_identifier_char((LastChar = advance())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "function") return tok_function;
    if (IdentifierStr == "return") return tok_return;
    if (IdentifierStr == "extern") return tok_extern;
    if (IdentifierStr == "if") return tok_if;
    if (IdentifierStr == "else") return tok_else;
    if (IdentifierStr == "for") return tok_for;
    if (IdentifierStr == "in") return tok_in;
    if (IdentifierStr == "binary") return tok_binary;
    if (IdentifierStr == "unary") return tok_unary;
    if (IdentifierStr == "var") return tok_var;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') {  // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = advance();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  if (LastChar == '#') {
    // Comment until end of line.
    do
      LastChar = advance();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF) return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF) return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = advance();
  return ThisChar;
}

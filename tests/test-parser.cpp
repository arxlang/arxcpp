#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "../src/io.h"
#include "../src/lexer.h"
#include "../src/parser.h"

TEST(ParserTest, GetNextTokenTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*) R""""(
  function math(x):
    if x > 10:
      x + 1
    else:
      x * 20

  math(1);
  )"""");

  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_function);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) '(');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ')');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ':');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_if);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) '>');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_number);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ':');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) '+');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_number);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_else);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ':');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) '*');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_number);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_identifier);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) '(');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, tok_number);
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ')');
  Lexer::getNextToken();
  EXPECT_EQ(Lexer::CurTok, (int) ';');
}

TEST(ParserTest, BinopPrecedenceTest) {
  Parser::setup();

  EXPECT_EQ(Parser::BinopPrecedence['='], 2);
  EXPECT_EQ(Parser::BinopPrecedence['<'], 10);
  EXPECT_EQ(Parser::BinopPrecedence['+'], 20);
  EXPECT_EQ(Parser::BinopPrecedence['-'], 20);
  EXPECT_EQ(Parser::BinopPrecedence['*'], 40);
}

TEST(ParserTest, ParseFloatExprTest) {
  /* Test gettok for main tokens */
  std::unique_ptr<FloatExprAST> expr;
  int tok;

  // TODO: check why it is necessary to add ; here
  string_to_buffer((char*) "1 2;");

  tok = Lexer::getNextToken();  // update Lexer::CurTok
  EXPECT_EQ(tok, tok_number);
  expr = Parser::ParseFloatExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 1);
  expr.reset();

  expr = Parser::ParseFloatExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 2);
  expr.reset();

  string_to_buffer((char*) "3");

  tok = Lexer::getNextToken();
  EXPECT_EQ(tok, tok_number);
  expr = Parser::ParseFloatExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 3);
  expr.reset();
}

TEST(ParserTest, ParseIfExprTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*) R""""(
  if 1 > 2:
    a = 1
  else:
    a = 2
  )"""");

  Lexer::getNextToken();  // update Lexer::CurTok
  auto expr = Parser::ParsePrimary();
}

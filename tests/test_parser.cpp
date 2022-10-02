#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "../src/io.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/settings.h"

extern int CurTok;
extern SourceLocation CurLoc;

TEST(ParserTest, GetNextTokenTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*)R""""(
  function math(x):
    if x > 10:
      x + 1
    else:
      x * 20

  math(1);
  )"""");

  getNextToken();
  EXPECT_EQ(CurTok, tok_function);
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)'(');
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)')');
  getNextToken();
  EXPECT_EQ(CurTok, (int)':');
  getNextToken();
  EXPECT_EQ(CurTok, tok_if);
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)'>');
  getNextToken();
  EXPECT_EQ(CurTok, tok_number);
  getNextToken();
  EXPECT_EQ(CurTok, (int)':');
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)'+');
  getNextToken();
  EXPECT_EQ(CurTok, tok_number);
  getNextToken();
  EXPECT_EQ(CurTok, tok_else);
  getNextToken();
  EXPECT_EQ(CurTok, (int)':');
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)'*');
  getNextToken();
  EXPECT_EQ(CurTok, tok_number);
  getNextToken();
  EXPECT_EQ(CurTok, tok_identifier);
  getNextToken();
  EXPECT_EQ(CurTok, (int)'(');
  getNextToken();
  EXPECT_EQ(CurTok, tok_number);
  getNextToken();
  EXPECT_EQ(CurTok, (int)')');
  getNextToken();
  EXPECT_EQ(CurTok, (int)';');
}

TEST(ParserTest, BinopPrecedenceTest) {
  load_settings();

  EXPECT_EQ(BinopPrecedence['='], 2);
  EXPECT_EQ(BinopPrecedence['<'], 10);
  EXPECT_EQ(BinopPrecedence['+'], 20);
  EXPECT_EQ(BinopPrecedence['-'], 20);
  EXPECT_EQ(BinopPrecedence['*'], 40);
}

TEST(ParserTest, ParseNumberExprTest) {
  /* Test gettok for main tokens */
  std::unique_ptr<NumberExprAST> expr;
  int tok;

  // TODO: check why it is necessary to add ; here
  string_to_buffer((char*)"1 2;");

  tok = getNextToken();  // update CurTok
  EXPECT_EQ(tok, tok_number);
  expr = ParseNumberExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 1);
  expr.reset();

  expr = ParseNumberExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 2);
  expr.reset();

  string_to_buffer((char*)"3");

  tok = getNextToken();
  EXPECT_EQ(tok, tok_number);
  expr = ParseNumberExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 3);
  expr.reset();
}

TEST(ParserTest, ParseIfExprTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*)R""""(
  if 1 > 2:
    a = 1
  else:
    a = 2
  )"""");

  getNextToken();  // update CurTok
  auto expr = ParsePrimary();
}

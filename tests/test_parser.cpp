#include <memory>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "../arx/include/input.h"
#include "../arx/include/lexer.h"
#include "../arx/include/parser.h"
#include "../arx/include/settings.h"

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

  string_to_buffer((char*)"1");

  getNextToken();  // update CurTok
  expr = ParseNumberExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 1);

  expr.reset();

  string_to_buffer((char*)"3");

  getNextToken();  // update CurTok
  // note: investigate why it is necessary to run it twice
  getNextToken();
  expr = ParseNumberExpr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->Val, 3);
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

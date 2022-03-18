#include <gtest/gtest.h>
#include "../arx/include/lexer.h"
#include "../arx/include/parser.h"
#include "../arx/include/settings.h"

extern int CurTok;

TEST(ParserTest, GetNextTokenTest) {
  /* Test gettok for main tokens */
  IOSource::content = (char*)R""""(
  function math(x):
    if x > 10:
      x + 1
    else:
      x * 20

  math(1);
  )"""";

  getNextToken();  // clean the buffer
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

  IOSource::content = nullptr;
}

TEST(ParserTest, BinopPrecedenceTest) {
  load_settings();

  EXPECT_EQ(BinopPrecedence['='], 2);
  EXPECT_EQ(BinopPrecedence['<'], 10);
  EXPECT_EQ(BinopPrecedence['+'], 20);
  EXPECT_EQ(BinopPrecedence['-'], 20);
  EXPECT_EQ(BinopPrecedence['*'], 40);
}

TEST(ParserTest, BinopPrecedenceTest) {
  load_settings();

  EXPECT_EQ(BinopPrecedence['='], 2);
  EXPECT_EQ(BinopPrecedence['<'], 10);
  EXPECT_EQ(BinopPrecedence['+'], 20);
  EXPECT_EQ(BinopPrecedence['-'], 20);
  EXPECT_EQ(BinopPrecedence['*'], 40);
}

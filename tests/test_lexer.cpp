#include <string>

#include <gtest/gtest.h>

#include "../arx/include/input.h"
#include "../arx/include/lexer.h"
#include "../arx/include/utils.h"

TEST(LexerTest, TokenNameTest) {
  /* Test some results from getTokName */
  EXPECT_EQ(getTokName(tok_eof), "eof");
  EXPECT_EQ(getTokName(tok_function), "function");
  EXPECT_EQ(getTokName(tok_return), "return");
  EXPECT_EQ(getTokName(tok_identifier), "identifier");
  EXPECT_EQ(getTokName(tok_if), "if");
  EXPECT_EQ(getTokName(tok_for), "for");
  EXPECT_EQ(getTokName('+'), "+");
}

TEST(LexerTest, AdvanceTest) {
  string_to_buffer((char*)"1");
  EXPECT_EQ(advance(), 49);

  string_to_buffer((char*)"2");
  EXPECT_EQ(advance(), 50);

  string_to_buffer((char*)"3");
  EXPECT_EQ(advance(), 51);
}

TEST(LexerTest, GetTokSimpleTest) {
  string_to_buffer((char*)"11");
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(NumVal, 11);

  string_to_buffer((char*)"21");
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(NumVal, 21);

  string_to_buffer((char*)"31");
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(NumVal, 31);
}

TEST(LexerTest, GetNextTokenSimpleTest) {
  string_to_buffer((char*)"11");
  EXPECT_EQ(getNextToken(), tok_number);
  EXPECT_EQ(NumVal, 11);

  string_to_buffer((char*)"21");
  EXPECT_EQ(getNextToken(), tok_number);
  EXPECT_EQ(NumVal, 21);

  string_to_buffer((char*)"31");
  EXPECT_EQ(getNextToken(), tok_number);
  EXPECT_EQ(NumVal, 31);
}

TEST(LexerTest, GetTokTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*)R""""(
  function math(x):
    if x > 10:
      x + 1
    else:
      x * 20

  math(1);
  )"""");

  EXPECT_EQ(gettok(), tok_function);
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)'(');
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)')');
  EXPECT_EQ(gettok(), (int)':');
  EXPECT_EQ(gettok(), tok_if);
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)'>');
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(gettok(), (int)':');
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)'+');
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(gettok(), tok_else);
  EXPECT_EQ(gettok(), (int)':');
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)'*');
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(gettok(), tok_identifier);
  EXPECT_EQ(gettok(), (int)'(');
  EXPECT_EQ(gettok(), tok_number);
  EXPECT_EQ(gettok(), (int)')');
  EXPECT_EQ(gettok(), (int)';');
}

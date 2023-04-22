#include <string>

#include <gtest/gtest.h>

#include "../src/io.h"
#include "../src/lexer.h"
#include "../src/utils.h"

TEST(LexerTest, TokenNameTest) {
  /* Test some results from get_tok_name */
  EXPECT_EQ(Lexer::get_tok_name(tok_eof), "eof");
  EXPECT_EQ(Lexer::get_tok_name(tok_function), "function");
  EXPECT_EQ(Lexer::get_tok_name(tok_return), "return");
  EXPECT_EQ(Lexer::get_tok_name(tok_identifier), "identifier");
  EXPECT_EQ(Lexer::get_tok_name(tok_if), "if");
  EXPECT_EQ(Lexer::get_tok_name(tok_for), "for");
  EXPECT_EQ(Lexer::get_tok_name('+'), "+");
}

TEST(LexerTest, AdvanceTest) {
  string_to_buffer((char*) "1");
  EXPECT_EQ(Lexer::advance(), 49);

  string_to_buffer((char*) "2");
  EXPECT_EQ(Lexer::advance(), 50);

  string_to_buffer((char*) "3");
  EXPECT_EQ(Lexer::advance(), 51);
}

TEST(LexerTest, GetTokSimpleTest) {
  string_to_buffer((char*) "11");
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 11);

  string_to_buffer((char*) "21");
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 21);

  string_to_buffer((char*) "31");
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 31);
}

TEST(LexerTest, GetNextTokenSimpleTest) {
  string_to_buffer((char*) "11");
  EXPECT_EQ(Lexer::get_next_token(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 11);

  string_to_buffer((char*) "21");
  EXPECT_EQ(Lexer::get_next_token(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 21);

  string_to_buffer((char*) "31");
  EXPECT_EQ(Lexer::get_next_token(), tok_float_literal);
  EXPECT_EQ(Lexer::num_float, 31);
}

TEST(LexerTest, GetTokTest) {
  /* Test gettok for main tokens */
  string_to_buffer((char*) R""""(
  fn math(x: float) -> float:
    if x > 10:
      return x + 1;
    else:
      return x * 20;

  math(1);
  )"""");

  EXPECT_EQ(Lexer::gettok(), tok_function);
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) '(');
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) ':');
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) ')');
  EXPECT_EQ(Lexer::gettok(), tok_arrow_right);
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) ':');
  EXPECT_EQ(Lexer::gettok(), tok_if);
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) '>');
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::gettok(), (int) ':');
  EXPECT_EQ(Lexer::gettok(), tok_return);
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) '+');
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::gettok(), (int) ';');
  EXPECT_EQ(Lexer::gettok(), tok_else);
  EXPECT_EQ(Lexer::gettok(), (int) ':');
  EXPECT_EQ(Lexer::gettok(), tok_return);
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) '*');
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::gettok(), (int) ';');
  EXPECT_EQ(Lexer::gettok(), tok_identifier);
  EXPECT_EQ(Lexer::gettok(), (int) '(');
  EXPECT_EQ(Lexer::gettok(), tok_float_literal);
  EXPECT_EQ(Lexer::gettok(), (int) ')');
  EXPECT_EQ(Lexer::gettok(), (int) ';');
}

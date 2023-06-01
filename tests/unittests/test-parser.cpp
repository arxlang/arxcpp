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

  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_function);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) '(');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ')');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ':');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_if);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) '>');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_float_literal);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ':');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) '+');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_float_literal);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_else);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ':');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) '*');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_float_literal);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_identifier);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) '(');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, tok_float_literal);
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ')');
  Lexer::get_next_token();
  EXPECT_EQ(Lexer::cur_tok, (int) ';');
}

TEST(ParserTest, BinopPrecedenceTest) {
  Parser::setup();

  EXPECT_EQ(Parser::bin_op_precedence['='], 2);
  EXPECT_EQ(Parser::bin_op_precedence['<'], 10);
  EXPECT_EQ(Parser::bin_op_precedence['+'], 20);
  EXPECT_EQ(Parser::bin_op_precedence['-'], 20);
  EXPECT_EQ(Parser::bin_op_precedence['*'], 40);
}

TEST(ParserTest, ParseFloatExprTest) {
  /* Test gettok for main tokens */
  std::unique_ptr<FloatExprAST> expr;
  int tok;

  // TODO: check why it is necessary to add ; here
  string_to_buffer((char*) "1 2;");

  tok = Lexer::get_next_token();  // update Lexer::cur_tok
  EXPECT_EQ(tok, tok_float_literal);
  expr = Parser::parse_float_expr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->val, 1);
  expr.reset();

  expr = Parser::parse_float_expr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->val, 2);
  expr.reset();

  string_to_buffer((char*) "3");

  tok = Lexer::get_next_token();
  EXPECT_EQ(tok, tok_float_literal);
  expr = Parser::parse_float_expr();
  EXPECT_NE(expr, nullptr);
  EXPECT_EQ(expr->val, 3);
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

  Lexer::get_next_token();  // update Lexer::cur_tok
  auto expr = Parser::parse_primary();
}

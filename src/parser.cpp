#include "parser.h"     // for ExprAST, Parser, PrototypeAST, ForExprAST
#include <cctype>       // for isascii
#include <cstring>      // for strcat, strcpy
#include <iostream>     // for operator<<, basic_ostream::operator<<, cout
#include <map>          // for map
#include <memory>       // for unique_ptr, make_unique
#include <string>       // for string, to_string
#include <type_traits>  // for __underlying_type_impl<>::type, underlying_type
#include <utility>      // for move, pair
#include <vector>       // for vector
#include "error.h"      // for LogError
#include "lexer.h"  // for Lexer, Lexer::cur_tok, Lexer::cur_loc, tok_iden...

/**
 * @brief This holds the precedence for each binary operator that
 * is defined.
 */

std::map<char, int> Parser::bin_op_precedence;

void ExprAST::accept(Visitor& visitor) {
  switch (this->kind) {
    case ExprKind::FloatDTKind: {
      visitor.visit((FloatExprAST&) *this);
      break;
    }
    case ExprKind::VariableKind: {
      visitor.visit((VariableExprAST&) *this);
      break;
    }
    case ExprKind::UnaryOpKind: {
      visitor.visit((UnaryExprAST&) *this);
      break;
    }
    case ExprKind::BinaryOpKind: {
      visitor.visit((BinaryExprAST&) *this);
      break;
    }
    case ExprKind::CallKind: {
      visitor.visit((CallExprAST&) *this);
      break;
    }
    case ExprKind::IfKind: {
      visitor.visit((IfExprAST&) *this);
      break;
    }
    case ExprKind::ForKind: {
      visitor.visit((ForExprAST&) *this);
      break;
    }
    case ExprKind::VarKind: {
      visitor.visit((VarExprAST&) *this);
      break;
    }
    case ExprKind::PrototypeKind: {
      visitor.visit((PrototypeAST&) *this);
      break;
    }
    case ExprKind::FunctionKind: {
      visitor.visit((FunctionAST&) *this);
      break;
    }
    case ExprKind::GenericKind: {
      std::cout << "[WW] Generic Kind doesn't have a downcasting";
    }
    default: {
      std::cout << static_cast<std::underlying_type<ExprKind>::type>(
                     this->kind)
                << std::endl;
      std::cout << "[WW] DOWNCASTING MATCH FAILED";
      visitor.clean();
    }
  };
}

/**
 * @brief Get the precedence of the pending binary operator token.
 * @return The token precedence.
 *
 */
auto Parser::get_tok_precedence() -> int {
  if (!isascii(Lexer::cur_tok)) {
    return -1;
  }

  // Make sure it's a declared binop.
  int tok_prec = Parser::bin_op_precedence[Lexer::cur_tok];
  if (tok_prec <= 0) {
    return -1;
  }
  return tok_prec;
}

/**
 * @brief Parse the number expression.
 * @return
 * numberexpr ::= number
 */
std::unique_ptr<FloatExprAST> Parser::parse_float_expr() {
  auto result = std::make_unique<FloatExprAST>(Lexer::num_float);
  Lexer::get_next_token();  // consume the number
  return result;
}

/**
 * @brief Parse the parenthesis expression.
 * @return
 * parenexpr ::= '(' expression ')'
 */
std::unique_ptr<ExprAST> Parser::parse_paren_expr() {
  Lexer::get_next_token();  // eat (.
  auto expr = Parser::parse_expression();
  if (!expr) {
    return nullptr;
  }

  if (Lexer::cur_tok != ')') {
    return LogError<ExprAST>("Parser: Expected ')'");
  }
  Lexer::get_next_token();  // eat ).
  return expr;
}

/**
 * @brief Parse the identifier expression.
 * @return
 * identifierexpr
 *   ::= identifier
 *   ::= identifier '(' expression* ')'
 */
std::unique_ptr<ExprAST> Parser::parse_identifier_expr() {
  std::string IdName = Lexer::identifier_str;

  SourceLocation LitLoc = Lexer::cur_loc;

  Lexer::get_next_token();  // eat identifier.

  if (Lexer::cur_tok != '(') {  // Simple variable ref.
    return std::make_unique<VariableExprAST>(LitLoc, IdName);
  }

  // Call. //
  Lexer::get_next_token();  // eat (
  std::vector<std::unique_ptr<ExprAST>> args;
  if (Lexer::cur_tok != ')') {
    while (true) {
      if (auto arg = Parser::parse_expression()) {
        args.emplace_back(std::move(arg));
      } else {
        return nullptr;
      }

      if (Lexer::cur_tok == ')') {
        break;
      }

      if (Lexer::cur_tok != ',') {
        return LogError<ExprAST>(
          "Parser: Expected ')' or ',' in argument list");
        Lexer::get_next_token();
      }
    }
  }

  // Eat the ')'.
  Lexer::get_next_token();

  return std::make_unique<CallExprAST>(LitLoc, IdName, std::move(args));
}

/**
 * @brief Parse the `if` expression.
 * @return
 * ifexpr ::= 'if' expression 'then' expression 'else' expression
 */
std::unique_ptr<IfExprAST> Parser::parse_if_expr() {
  SourceLocation if_loc = Lexer::cur_loc;
  char msg[80];

  Lexer::get_next_token();  // eat the if.

  // condition.
  auto cond = Parser::parse_expression();
  if (!cond) {
    return nullptr;
  };

  if (Lexer::cur_tok != ':') {
    strcpy(msg, "Parser: `if` statement expected ':', received: '");
    strcat(msg, std::to_string(Lexer::cur_tok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  Lexer::get_next_token();  // eat the ':'

  auto then = Parser::parse_expression();
  if (!then) {
    return nullptr;
  };

  if (Lexer::cur_tok != tok_else) {
    return LogError<IfExprAST>("Parser: Expected else");
  }
  Lexer::get_next_token();  // eat the else token

  if (Lexer::cur_tok != ':') {
    strcpy(msg, "Parser: `else` statement expected ':', received: '");
    strcat(msg, std::to_string(Lexer::cur_tok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  Lexer::get_next_token();  // eat the ':'

  auto else_ = Parser::parse_expression();
  if (!else_) {
    return nullptr;
  };

  return std::make_unique<IfExprAST>(
    if_loc, std::move(cond), std::move(then), std::move(else_));
}

/**
 * @brief Parse the `for` expression.
 * @return
 * forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
 */
std::unique_ptr<ForExprAST> Parser::parse_for_expr() {
  Lexer::get_next_token();  // eat the for.

  if (Lexer::cur_tok != tok_identifier) {
    return LogError<ForExprAST>("Parser: Expected identifier after for");
  }

  std::string IdName = Lexer::identifier_str;
  Lexer::get_next_token();  // eat identifier.

  if (Lexer::cur_tok != '=') {
    return LogError<ForExprAST>("Parser: Expected '=' after for");
  }
  Lexer::get_next_token();  // eat '='.

  auto start = Parser::parse_expression();
  if (!start) {
    return nullptr;
  }
  if (Lexer::cur_tok != ',') {
    return LogError<ForExprAST>("Parser: Expected ',' after for start value");
  }
  Lexer::get_next_token();

  auto end = Parser::parse_expression();
  if (!end) {
    return nullptr;
  }

  // The step value is optional. //
  std::unique_ptr<ExprAST> step;
  if (Lexer::cur_tok == ',') {
    Lexer::get_next_token();
    step = Parser::parse_expression();
    if (!step) {
      return nullptr;
    }
  }

  if (Lexer::cur_tok != tok_in) {
    return LogError<ForExprAST>("Parser: Expected 'in' after for");
  }
  Lexer::get_next_token();  // eat 'in'.

  auto body = Parser::parse_expression();
  if (!body) {
    return nullptr;
  }

  return std::make_unique<ForExprAST>(
    IdName,
    std::move(start),
    std::move(end),
    std::move(step),
    std::move(body));
}

/**
 * @brief Parse the `var` declaration expression.
 * @return
 * varexpr ::= 'var' identifier ('=' expression)?
 *                    (',' identifier ('=' expression)?)* 'in' expression
 */
std::unique_ptr<VarExprAST> Parser::parse_var_expr() {
  Lexer::get_next_token();  // eat the var.

  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> var_names;

  // At least one variable name is required. //
  if (Lexer::cur_tok != tok_identifier) {
    return LogError<VarExprAST>("Parser: Expected identifier after var");
  }

  while (true) {
    std::string name = Lexer::identifier_str;
    Lexer::get_next_token();  // eat identifier.

    // Read the optional initializer. //
    std::unique_ptr<ExprAST> Init = nullptr;
    if (Lexer::cur_tok == '=') {
      Lexer::get_next_token();  // eat the '='.

      Init = Parser::parse_expression();
      if (!Init) {
        return nullptr;
      }
    }

    var_names.emplace_back(name, std::move(Init));

    // end of var list, exit loop. //
    if (Lexer::cur_tok != ',') {
      break;
    }
    Lexer::get_next_token();  // eat the ','.

    if (Lexer::cur_tok != tok_identifier) {
      return LogError<VarExprAST>(
        "Parser: Expected identifier list after var");
    }
  }

  // At this point, we have to have 'in'. //
  if (Lexer::cur_tok != tok_in) {
    return LogError<VarExprAST>("Parser: Expected 'in' keyword after 'var'");
  }
  Lexer::get_next_token();  // eat 'in'.

  auto body = Parser::parse_expression();
  if (!body) {
    return nullptr;
  }

  return std::make_unique<VarExprAST>(std::move(var_names), std::move(body));
}

/**
 * @brief Parse the primary expression.
 * @return
 * primary
 *   ::= identifierexpr
 *   ::= numberexpr
 *   ::= parenexpr
 *   ::= ifexpr
 *   ::= forexpr
 *   ::= varexpr
 */
std::unique_ptr<ExprAST> Parser::parse_primary() {
  char msg[80];

  switch (Lexer::cur_tok) {
    case tok_identifier:
      return Parser::parse_identifier_expr();
    case tok_float_literal:
      return static_cast<std::unique_ptr<ExprAST>>(parse_float_expr());
    case '(':
      return Parser::parse_paren_expr();
    case tok_if:
      return static_cast<std::unique_ptr<ExprAST>>(parse_if_expr());
    case tok_for:
      return static_cast<std::unique_ptr<ExprAST>>(parse_for_expr());
    case tok_var:
      return static_cast<std::unique_ptr<ExprAST>>(parse_var_expr());
    case ';':
      // ignore top-level semicolons.
      Lexer::get_next_token();  // eat `;`
      return Parser::parse_primary();
    default:
      strcpy(msg, "Parser: Unknown token when expecting an expression: '");
      strcat(msg, std::to_string(Lexer::cur_tok).c_str());
      strcat(msg, "'.");
      return LogError<ExprAST>(msg);
  }
}

/**
 * @brief Parse a unary expression.
 * @return
 * unary
 *   ::= primary
 *   ::= '!' unary
 */
std::unique_ptr<ExprAST> Parser::parse_unary() {
  // If the current token is not an operator, it must be a primary expr.
  if (
    !isascii(Lexer::cur_tok) || Lexer::cur_tok == '(' ||
    Lexer::cur_tok == ',') {
    return Parser::parse_primary();
  }

  // If this is a unary operator, read it.
  int Opc = Lexer::cur_tok;
  Lexer::get_next_token();
  if (auto operand = Parser::parse_unary()) {
    return std::make_unique<UnaryExprAST>(Opc, std::move(operand));
  }
  return nullptr;
}

/**
 * @brief Parse a binary expression.
 * @param expr_prec Expression Pression (deprecated)
 * @param lhs Left hand side Expression
 * @return
 * binoprhs
 *   ::= ('+' unary)*
 */
std::unique_ptr<ExprAST> Parser::parse_bin_op_rhs(
  int expr_prec, std::unique_ptr<ExprAST> lhs) {
  // If this is a binop, find its precedence. //
  while (true) {
    int tok_prec = get_tok_precedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (tok_prec < expr_prec) {
      return lhs;
    }

    // Okay, we know this is a binop.
    int BinOp = Lexer::cur_tok;
    SourceLocation BinLoc = Lexer::cur_loc;
    Lexer::get_next_token();  // eat binop

    // Parse the unary expression after the binary operator.
    auto rhs = Parser::parse_unary();
    if (!rhs) {
      return nullptr;
    }

    // If BinOp binds less tightly with rhs than the operator after rhs, let
    // the pending operator take rhs as its lhs.
    int next_prec = get_tok_precedence();
    if (tok_prec < next_prec) {
      rhs = Parser::parse_bin_op_rhs(tok_prec + 1, std::move(rhs));
      if (!rhs) {
        return nullptr;
      }
    }

    // Merge lhs/rhs.
    lhs = std::make_unique<BinaryExprAST>(
      BinLoc, BinOp, std::move(lhs), std::move(rhs));
  }
}

/**
 * @brief Parse an expression.
 * @return
 * expression
 *   ::= unary binoprhs
 *
 */
std::unique_ptr<ExprAST> Parser::parse_expression() {
  auto lhs = Parser::parse_unary();
  if (!lhs) {
    return nullptr;
  }

  return Parser::parse_bin_op_rhs(0, std::move(lhs));
}

/**
 * @brief Parse an extern prototype expression.
 * @return
 * prototype
 *   ::= id '(' id* ')'
 *   ::= binary LETTER number? (id, id)
 *   ::= unary LETTER (id)
 */
std::unique_ptr<PrototypeAST> Parser::parse_extern_prototype() {
  std::string fn_name;
  SourceLocation fn_loc = Lexer::cur_loc;

  switch (Lexer::cur_tok) {
    case tok_identifier:
      fn_name = Lexer::identifier_str;
      Lexer::get_next_token();
      break;

    default:
      return LogError<PrototypeAST>(
        "Parser: Expected function name in prototype");
  }

  if (Lexer::cur_tok != '(') {
    return LogError<PrototypeAST>(
      "Parser: Expected '(' in the function definition.");
  }

  std::vector<std::unique_ptr<VariableExprAST>> args;
  while (Lexer::get_next_token() == tok_identifier) {
    auto arg = std::make_unique<VariableExprAST>(
      VariableExprAST(Lexer::cur_loc, Lexer::identifier_str));
    args.emplace_back(std::move(arg));
  }
  if (Lexer::cur_tok != ')') {
    return LogError<PrototypeAST>(
      "Parser: Expected ')' in the function definition.");
  }

  // success. //
  Lexer::get_next_token();  // eat ')'.

  return std::make_unique<PrototypeAST>(fn_loc, fn_name, std::move(args));
}

/**
 * @brief Parse the prototype expression.
 * @return
 * prototype
 *   ::= id '(' id* ')'
 *   ::= binary LETTER number? (id, id)
 *   ::= unary LETTER (id)
 */
std::unique_ptr<PrototypeAST> Parser::parse_prototype() {
  std::string fn_name;
  SourceLocation fn_loc = Lexer::cur_loc;

  switch (Lexer::cur_tok) {
    case tok_identifier:
      fn_name = Lexer::identifier_str;
      Lexer::get_next_token();
      break;

    default:
      return LogError<PrototypeAST>(
        "Parser: Expected function name in prototype");
  }

  if (Lexer::cur_tok != '(') {
    return LogError<PrototypeAST>(
      "Parser: Expected '(' in the function definition.");
  }

  std::vector<std::unique_ptr<VariableExprAST>> args;
  while (Lexer::get_next_token() == tok_identifier) {
    auto arg = std::make_unique<VariableExprAST>(
      VariableExprAST(Lexer::cur_loc, Lexer::identifier_str));
    args.emplace_back(std::move(arg));
  }
  if (Lexer::cur_tok != ')') {
    return LogError<PrototypeAST>(
      "Parser: Expected ')' in the function definition.");
  }

  // success. //
  Lexer::get_next_token();  // eat ')'.

  if (Lexer::cur_tok != ':') {
    return LogError<PrototypeAST>(
      "Parser: Expected ':' in the function definition");
  }

  Lexer::get_next_token();  // eat ':'.

  return std::make_unique<PrototypeAST>(fn_loc, fn_name, std::move(args));
}

/**
 * @brief Parse the function definition expression.
 * @return
 * definition ::= 'function' prototype expression
 */
std::unique_ptr<FunctionAST> Parser::parse_definition() {
  Lexer::get_next_token();  // eat function.
  auto proto = Parser::parse_prototype();
  if (!proto) {
    return nullptr;
  }

  if (auto E = Parser::parse_expression()) {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief Parse the top level expression.
 * @return
 * toplevelexpr ::= expression
 */
std::unique_ptr<FunctionAST> Parser::parse_top_level_expr() {
  SourceLocation fn_loc = Lexer::cur_loc;
  if (auto E = Parser::parse_expression()) {
    // Make an anonymous proto.
    auto proto = std::make_unique<PrototypeAST>(
      fn_loc,
      "__anon_expr",
      std::move(std::vector<std::unique_ptr<VariableExprAST>>()));
    return std::make_unique<FunctionAST>(std::move(proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief Parse the extern expression;
 * @return
 * external ::= 'extern' prototype
 */
std::unique_ptr<PrototypeAST> Parser::parse_extern() {
  Lexer::get_next_token();  // eat extern.
  return Parser::parse_extern_prototype();
}

auto Parser::parse() -> std::unique_ptr<TreeAST> {
  auto ast = std::make_unique<TreeAST>(TreeAST());

  while (true) {
    std::unique_ptr<ExprAST> node_uptr = nullptr;

    switch (Lexer::cur_tok) {
      case tok_eof:
        return std::move(ast);
      case tok_not_initialized:
        Lexer::get_next_token();
        break;
      case ';':
        Lexer::get_next_token();
        // ignore top-level semicolons.
        break;
      case tok_function:
        ast->nodes.emplace_back(Parser::parse_definition());
        break;
      case tok_extern:
        ast->nodes.emplace_back(Parser::parse_extern());
        break;
      default:
        ast->nodes.emplace_back(Parser::parse_top_level_expr());
        break;
    }
  }
}

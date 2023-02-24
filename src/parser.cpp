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
#include "lexer.h"      // for Lexer, Lexer::CurTok, Lexer::CurLoc, tok_iden...

/**
 * @brief This holds the precedence for each binary operator that
 * is defined.
 */

std::map<char, int> Parser::BinopPrecedence;

void ExprAST::accept(Visitor* visitor) {
  switch (this->kind) {
    case ExprKind::NumberKind: {
      visitor->visit((NumberExprAST*) this);
      break;
    }
    case ExprKind::VariableKind: {
      visitor->visit((VariableExprAST*) this);
      break;
    }
    case ExprKind::UnaryKind: {
      visitor->visit((UnaryExprAST*) this);
      break;
    }
    case ExprKind::BinaryKind: {
      visitor->visit((BinaryExprAST*) this);
      break;
    }
    case ExprKind::CallKind: {
      visitor->visit((CallExprAST*) this);
      break;
    }
    case ExprKind::IfKind: {
      visitor->visit((IfExprAST*) this);
      break;
    }
    case ExprKind::ForKind: {
      visitor->visit((ForExprAST*) this);
      break;
    }
    case ExprKind::VarKind: {
      visitor->visit((VarExprAST*) this);
      break;
    }
    case ExprKind::PrototypeKind: {
      visitor->visit((PrototypeAST*) this);
      break;
    }
    case ExprKind::FunctionKind: {
      visitor->visit((FunctionAST*) this);
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
      visitor->clean();
    }
  };
}

/**
 * @brief Get the precedence of the pending binary operator token.
 * @return The token precedence.
 *
 */
auto Parser::GetTokPrecedence() -> int {
  if (!isascii(Lexer::CurTok)) {
    return -1;
  }

  // Make sure it's a declared binop.
  int TokPrec = Parser::BinopPrecedence[Lexer::CurTok];
  if (TokPrec <= 0) {
    return -1;
  }
  return TokPrec;
}

/**
 * @brief Parse the number expression.
 * @return
 * numberexpr ::= number
 */
std::unique_ptr<NumberExprAST> Parser::ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(Lexer::NumVal);
  Lexer::getNextToken();  // consume the number
  return Result;
}

/**
 * @brief Parse the parenthesis expression.
 * @return
 * parenexpr ::= '(' expression ')'
 */
std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
  Lexer::getNextToken();  // eat (.
  auto V = Parser::ParseExpression();
  if (!V) {
    return nullptr;
  }

  if (Lexer::CurTok != ')') {
    return LogError<ExprAST>("Parser: Expected ')'");
  }
  Lexer::getNextToken();  // eat ).
  return V;
}

/**
 * @brief Parse the identifier expression.
 * @return
 * identifierexpr
 *   ::= identifier
 *   ::= identifier '(' expression* ')'
 */
std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
  std::string IdName = Lexer::IdentifierStr;

  SourceLocation LitLoc = Lexer::CurLoc;

  Lexer::getNextToken();  // eat identifier.

  if (Lexer::CurTok != '(') {  // Simple variable ref.
    return std::make_unique<VariableExprAST>(LitLoc, IdName);
  }

  // Call. //
  Lexer::getNextToken();  // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (Lexer::CurTok != ')') {
    while (true) {
      if (auto Arg = Parser::ParseExpression()) {
        Args.push_back(std::move(Arg));
      } else {
        return nullptr;
      }

      if (Lexer::CurTok == ')') {
        break;
      }

      if (Lexer::CurTok != ',') {
        return LogError<ExprAST>(
          "Parser: Expected ')' or ',' in argument list");
        Lexer::getNextToken();
      }
    }
  }

  // Eat the ')'.
  Lexer::getNextToken();

  return std::make_unique<CallExprAST>(LitLoc, IdName, std::move(Args));
}

/**
 * @brief Parse the `if` expression.
 * @return
 * ifexpr ::= 'if' expression 'then' expression 'else' expression
 */
std::unique_ptr<IfExprAST> Parser::ParseIfExpr() {
  SourceLocation IfLoc = Lexer::CurLoc;
  char msg[80];

  Lexer::getNextToken();  // eat the if.

  // condition.
  auto Cond = Parser::ParseExpression();
  if (!Cond) {
    return nullptr;
  };

  if (Lexer::CurTok != ':') {
    strcpy(msg, "Parser: `if` statement expected ':', received: '");
    strcat(msg, std::to_string(Lexer::CurTok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  Lexer::getNextToken();  // eat the ':'

  auto Then = Parser::ParseExpression();
  if (!Then) {
    return nullptr;
  };

  if (Lexer::CurTok != tok_else) {
    return LogError<IfExprAST>("Parser: Expected else");
  }
  Lexer::getNextToken();  // eat the else token

  if (Lexer::CurTok != ':') {
    strcpy(msg, "Parser: `else` statement expected ':', received: '");
    strcat(msg, std::to_string(Lexer::CurTok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  Lexer::getNextToken();  // eat the ':'

  auto Else = Parser::ParseExpression();
  if (!Else) {
    return nullptr;
  };

  return std::make_unique<IfExprAST>(
    IfLoc, std::move(Cond), std::move(Then), std::move(Else));
}

/**
 * @brief Parse the `for` expression.
 * @return
 * forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
 */
std::unique_ptr<ForExprAST> Parser::ParseForExpr() {
  Lexer::getNextToken();  // eat the for.

  if (Lexer::CurTok != tok_identifier) {
    return LogError<ForExprAST>("Parser: Expected identifier after for");
  }

  std::string IdName = Lexer::IdentifierStr;
  Lexer::getNextToken();  // eat identifier.

  if (Lexer::CurTok != '=') {
    return LogError<ForExprAST>("Parser: Expected '=' after for");
  }
  Lexer::getNextToken();  // eat '='.

  auto Start = Parser::ParseExpression();
  if (!Start) {
    return nullptr;
  }
  if (Lexer::CurTok != ',') {
    return LogError<ForExprAST>("Parser: Expected ',' after for start value");
  }
  Lexer::getNextToken();

  auto End = Parser::ParseExpression();
  if (!End) {
    return nullptr;
  }

  // The step value is optional. //
  std::unique_ptr<ExprAST> Step;
  if (Lexer::CurTok == ',') {
    Lexer::getNextToken();
    Step = Parser::ParseExpression();
    if (!Step) {
      return nullptr;
    }
  }

  if (Lexer::CurTok != tok_in) {
    return LogError<ForExprAST>("Parser: Expected 'in' after for");
  }
  Lexer::getNextToken();  // eat 'in'.

  auto Body = Parser::ParseExpression();
  if (!Body) {
    return nullptr;
  }

  return std::make_unique<ForExprAST>(
    IdName,
    std::move(Start),
    std::move(End),
    std::move(Step),
    std::move(Body));
}

/**
 * @brief Parse the `var` declaration expression.
 * @return
 * varexpr ::= 'var' identifier ('=' expression)?
 *                    (',' identifier ('=' expression)?)* 'in' expression
 */
std::unique_ptr<VarExprAST> Parser::ParseVarExpr() {
  Lexer::getNextToken();  // eat the var.

  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

  // At least one variable name is required. //
  if (Lexer::CurTok != tok_identifier) {
    return LogError<VarExprAST>("Parser: Expected identifier after var");
  }

  while (true) {
    std::string Name = Lexer::IdentifierStr;
    Lexer::getNextToken();  // eat identifier.

    // Read the optional initializer. //
    std::unique_ptr<ExprAST> Init = nullptr;
    if (Lexer::CurTok == '=') {
      Lexer::getNextToken();  // eat the '='.

      Init = Parser::ParseExpression();
      if (!Init) {
        return nullptr;
      }
    }

    VarNames.emplace_back(Name, std::move(Init));

    // End of var list, exit loop. //
    if (Lexer::CurTok != ',') {
      break;
    }
    Lexer::getNextToken();  // eat the ','.

    if (Lexer::CurTok != tok_identifier) {
      return LogError<VarExprAST>(
        "Parser: Expected identifier list after var");
    }
  }

  // At this point, we have to have 'in'. //
  if (Lexer::CurTok != tok_in) {
    return LogError<VarExprAST>("Parser: Expected 'in' keyword after 'var'");
  }
  Lexer::getNextToken();  // eat 'in'.

  auto Body = Parser::ParseExpression();
  if (!Body) {
    return nullptr;
  }

  return std::make_unique<VarExprAST>(std::move(VarNames), std::move(Body));
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
std::unique_ptr<ExprAST> Parser::ParsePrimary() {
  char msg[80];

  switch (Lexer::CurTok) {
    case tok_identifier:
      return Parser::ParseIdentifierExpr();
    case tok_number:
      return static_cast<std::unique_ptr<ExprAST>>(ParseNumberExpr());
    case '(':
      return Parser::ParseParenExpr();
    case tok_if:
      return static_cast<std::unique_ptr<ExprAST>>(ParseIfExpr());
    case tok_for:
      return static_cast<std::unique_ptr<ExprAST>>(ParseForExpr());
    case tok_var:
      return static_cast<std::unique_ptr<ExprAST>>(ParseVarExpr());
    case ';':
      // ignore top-level semicolons.
      Lexer::getNextToken();  // eat `;`
      return Parser::ParsePrimary();
    default:
      strcpy(msg, "Parser: Unknown token when expecting an expression: '");
      strcat(msg, std::to_string(Lexer::CurTok).c_str());
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
std::unique_ptr<ExprAST> Parser::ParseUnary() {
  // If the current token is not an operator, it must be a primary expr.
  if (
    !isascii(Lexer::CurTok) || Lexer::CurTok == '(' || Lexer::CurTok == ',') {
    return Parser::ParsePrimary();
  }

  // If this is a unary operator, read it.
  int Opc = Lexer::CurTok;
  Lexer::getNextToken();
  if (auto Operand = Parser::ParseUnary()) {
    return std::make_unique<UnaryExprAST>(Opc, std::move(Operand));
  }
  return nullptr;
}

/**
 * @brief Parse a binary expression.
 * @param ExprPrec Expression Pression (deprecated)
 * @param LHS Left hand side Expression
 * @return
 * binoprhs
 *   ::= ('+' unary)*
 */
std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(
  int ExprPrec, std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence. //
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec) {
      return LHS;
    }

    // Okay, we know this is a binop.
    int BinOp = Lexer::CurTok;
    SourceLocation BinLoc = Lexer::CurLoc;
    Lexer::getNextToken();  // eat binop

    // Parse the unary expression after the binary operator.
    auto RHS = Parser::ParseUnary();
    if (!RHS) {
      return nullptr;
    }

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = Parser::ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS) {
        return nullptr;
      }
    }

    // Merge LHS/RHS.
    LHS = std::make_unique<BinaryExprAST>(
      BinLoc, BinOp, std::move(LHS), std::move(RHS));
  }
}

/**
 * @brief Parse an expression.
 * @return
 * expression
 *   ::= unary binoprhs
 *
 */
std::unique_ptr<ExprAST> Parser::ParseExpression() {
  auto LHS = Parser::ParseUnary();
  if (!LHS) {
    return nullptr;
  }

  return Parser::ParseBinOpRHS(0, std::move(LHS));
}

/**
 * @brief Parse an extern prototype expression.
 * @return
 * prototype
 *   ::= id '(' id* ')'
 *   ::= binary LETTER number? (id, id)
 *   ::= unary LETTER (id)
 */
std::unique_ptr<PrototypeAST> Parser::ParseExternPrototype() {
  std::string FnName;
  SourceLocation FnLoc = Lexer::CurLoc;

  switch (Lexer::CurTok) {
    case tok_identifier:
      FnName = Lexer::IdentifierStr;
      Lexer::getNextToken();
      break;

    default:
      return LogError<PrototypeAST>(
        "Parser: Expected function name in prototype");
  }

  if (Lexer::CurTok != '(') {
    return LogError<PrototypeAST>(
      "Parser: Expected '(' in the function definition.");
  }

  std::vector<VariableExprAST*> Args;
  while (Lexer::getNextToken() == tok_identifier) {
    Args.push_back(new VariableExprAST(Lexer::CurLoc, Lexer::IdentifierStr));
  }
  if (Lexer::CurTok != ')') {
    return LogError<PrototypeAST>(
      "Parser: Expected ')' in the function definition.");
  }

  // success. //
  Lexer::getNextToken();  // eat ')'.

  return std::make_unique<PrototypeAST>(FnLoc, FnName, Args);
}

/**
 * @brief Parse the prototype expression.
 * @return
 * prototype
 *   ::= id '(' id* ')'
 *   ::= binary LETTER number? (id, id)
 *   ::= unary LETTER (id)
 */
std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
  std::string FnName;
  SourceLocation FnLoc = Lexer::CurLoc;

  switch (Lexer::CurTok) {
    case tok_identifier:
      FnName = Lexer::IdentifierStr;
      Lexer::getNextToken();
      break;

    default:
      return LogError<PrototypeAST>(
        "Parser: Expected function name in prototype");
  }

  if (Lexer::CurTok != '(') {
    return LogError<PrototypeAST>(
      "Parser: Expected '(' in the function definition.");
  }

  std::vector<VariableExprAST*> Args;
  while (Lexer::getNextToken() == tok_identifier) {
    Args.push_back(new VariableExprAST(Lexer::CurLoc, Lexer::IdentifierStr));
  }
  if (Lexer::CurTok != ')') {
    return LogError<PrototypeAST>(
      "Parser: Expected ')' in the function definition.");
  }

  // success. //
  Lexer::getNextToken();  // eat ')'.

  if (Lexer::CurTok != ':') {
    return LogError<PrototypeAST>(
      "Parser: Expected ':' in the function definition");
  }

  Lexer::getNextToken();  // eat ':'.

  return std::make_unique<PrototypeAST>(FnLoc, FnName, Args);
}

/**
 * @brief Parse the function definition expression.
 * @return
 * definition ::= 'function' prototype expression
 */
std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
  Lexer::getNextToken();  // eat function.
  auto Proto = Parser::ParsePrototype();
  if (!Proto) {
    return nullptr;
  }

  if (auto E = Parser::ParseExpression()) {
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief Parse the top level expression.
 * @return
 * toplevelexpr ::= expression
 */
std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
  SourceLocation FnLoc = Lexer::CurLoc;
  if (auto E = Parser::ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>(
      FnLoc, "__anon_expr", std::vector<VariableExprAST*>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief Parse the extern expression;
 * @return
 * external ::= 'extern' prototype
 */
std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
  Lexer::getNextToken();  // eat extern.
  return Parser::ParseExternPrototype();
}

auto Parser::parse() -> TreeAST* {
  TreeAST* ast = new TreeAST();

  while (true) {
    std::unique_ptr<ExprAST> node_uptr = nullptr;

    switch (Lexer::CurTok) {
      case tok_eof:
        return ast;
      case tok_not_initialized:
        Lexer::getNextToken();
        break;
      case ';':
        Lexer::getNextToken();
        // ignore top-level semicolons.
        break;
      case tok_function:
        ast->nodes.emplace_back(Parser::ParseDefinition());
        break;
      case tok_extern:
        ast->nodes.emplace_back(Parser::ParseExtern());
        break;
      default:
        ast->nodes.emplace_back(Parser::ParseTopLevelExpr());
        break;
    }
  }
}

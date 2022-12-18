#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "error.h"
#include "lexer.h"
#include "parser.h"

extern SourceLocation CurLoc;
extern std::string IdentifierStr;
extern double NumVal;

extern int CurTok;

/**
 * @brief This holds the precedence for each binary operator that
 * is defined.
 */
std::map<char, int> BinopPrecedence;

/**
 * @brief Get the precedence of the pending binary operator token.
 * @return
 *
 */
auto GetTokPrecedence() -> int {
  if (!isascii(CurTok)) {
    return -1;
  }

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0) {
    return -1;
  }
  return TokPrec;
}

/**
 * @brief
 * @return
 * numberexpr ::= number
 */
std::unique_ptr<NumberExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken();  // consume the number
  return std::move(Result);
}

/**
 * @brief
 * @return
 * parenexpr ::= '(' expression ')'
 */
std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken();  // eat (.
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }

  if (CurTok != ')') {
    return LogError<ExprAST>("expected ')'");
  }
  getNextToken();  // eat ).
  return V;
}

/**
 * @brief
 * @return
 * identifierexpr
 *   ::= identifier
 *   ::= identifier '(' expression* ')'
 */
std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;

  SourceLocation LitLoc = CurLoc;

  getNextToken();  // eat identifier.

  if (CurTok != '(') {  // Simple variable ref.
    return std::make_unique<VariableExprAST>(LitLoc, IdName);
  }

  // Call. //
  getNextToken();  // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression()) {
        Args.push_back(std::move(Arg));
      } else {
        return nullptr;
      }

      if (CurTok == ')') {
        break;
      }

      if (CurTok != ',') {
        return LogError<ExprAST>("Expected ')' or ',' in argument list");
        getNextToken();
      }
    }
  }

  // Eat the ')'.
  getNextToken();

  return std::make_unique<CallExprAST>(LitLoc, IdName, std::move(Args));
}

/**
 * @brief
 * @return
 * ifexpr ::= 'if' expression 'then' expression 'else' expression
 */
std::unique_ptr<IfExprAST> ParseIfExpr() {
  SourceLocation IfLoc = CurLoc;
  char msg[80];

  getNextToken();  // eat the if.

  // condition.
  auto Cond = ParseExpression();
  if (!Cond) {
    return nullptr;
  };

  if (CurTok != ':') {
    strcpy(msg, "`if` statement expected ':', received: '");
    strcat(msg, std::to_string(CurTok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  getNextToken();  // eat the ':'

  auto Then = ParseExpression();
  if (!Then) {
    return nullptr;
  };

  if (CurTok != tok_else) {
    return LogError<IfExprAST>("expected else");
  }
  getNextToken();  // eat the else token

  if (CurTok != ':') {
    strcpy(msg, "`else` statement expected ':', received: '");
    strcat(msg, std::to_string(CurTok).c_str());
    strcat(msg, "'.");
    return LogError<IfExprAST>(msg);
  }
  getNextToken();  // eat the ':'

  auto Else = ParseExpression();
  if (!Else) {
    return nullptr;
  };

  return std::make_unique<IfExprAST>(
    IfLoc, std::move(Cond), std::move(Then), std::move(Else));
}

/**
 * @brief
 * @return
 * forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
 */
std::unique_ptr<ForExprAST> ParseForExpr() {
  getNextToken();  // eat the for.

  if (CurTok != tok_identifier) {
    return LogError<ForExprAST>("expected identifier after for");
  }

  std::string IdName = IdentifierStr;
  getNextToken();  // eat identifier.

  if (CurTok != '=') {
    return LogError<ForExprAST>("expected '=' after for");
  }
  getNextToken();  // eat '='.

  auto Start = ParseExpression();
  if (!Start) {
    return nullptr;
  }
  if (CurTok != ',') {
    return LogError<ForExprAST>("expected ',' after for start value");
  }
  getNextToken();

  auto End = ParseExpression();
  if (!End) {
    return nullptr;
  }

  // The step value is optional. //
  std::unique_ptr<ExprAST> Step;
  if (CurTok == ',') {
    getNextToken();
    Step = ParseExpression();
    if (!Step) {
      return nullptr;
    }
  }

  if (CurTok != tok_in) {
    return LogError<ForExprAST>("expected 'in' after for");
  }
  getNextToken();  // eat 'in'.

  auto Body = ParseExpression();
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
 * @brief
 * @return
 * varexpr ::= 'var' identifier ('=' expression)?
 *                    (',' identifier ('=' expression)?)* 'in' expression
 */
std::unique_ptr<VarExprAST> ParseVarExpr() {
  getNextToken();  // eat the var.

  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

  // At least one variable name is required. //
  if (CurTok != tok_identifier) {
    return LogError<VarExprAST>("expected identifier after var");
  }

  while (true) {
    std::string Name = IdentifierStr;
    getNextToken();  // eat identifier.

    // Read the optional initializer. //
    std::unique_ptr<ExprAST> Init = nullptr;
    if (CurTok == '=') {
      getNextToken();  // eat the '='.

      Init = ParseExpression();
      if (!Init) {
        return nullptr;
      }
    }

    VarNames.emplace_back(Name, std::move(Init));

    // End of var list, exit loop. //
    if (CurTok != ',') {
      break;
    }
    getNextToken();  // eat the ','.

    if (CurTok != tok_identifier) {
      return LogError<VarExprAST>("expected identifier list after var");
    }
  }

  // At this point, we have to have 'in'. //
  if (CurTok != tok_in) {
    return LogError<VarExprAST>("expected 'in' keyword after 'var'");
  }
  getNextToken();  // eat 'in'.

  auto Body = ParseExpression();
  if (!Body) {
    return nullptr;
  }

  return std::make_unique<VarExprAST>(std::move(VarNames), std::move(Body));
}

/**
 * @brief
 * @return
 * primary
 *   ::= identifierexpr
 *   ::= numberexpr
 *   ::= parenexpr
 *   ::= ifexpr
 *   ::= forexpr
 *   ::= varexpr
 */
std::unique_ptr<ExprAST> ParsePrimary() {
  char msg[80];

  switch (CurTok) {
    case tok_identifier:
      return ParseIdentifierExpr();
    case tok_number:
      return static_cast<std::unique_ptr<ExprAST>>(ParseNumberExpr());
    case '(':
      return ParseParenExpr();
    case tok_if:
      return static_cast<std::unique_ptr<ExprAST>>(ParseIfExpr());
    case tok_for:
      return static_cast<std::unique_ptr<ExprAST>>(ParseForExpr());
    case tok_var:
      return static_cast<std::unique_ptr<ExprAST>>(ParseVarExpr());
    case ';':
      // ignore top-level semicolons.
      getNextToken();  // eat `;`
      return ParsePrimary();
    default:
      strcpy(msg, "Unknown token when expecting an expression: '");
      strcat(msg, std::to_string(CurTok).c_str());
      strcat(msg, "'.");
      return LogError<ExprAST>(msg);
  }
}

/**
 * @brief
 * @return
 * unary
 *   ::= primary
 *   ::= '!' unary
 */
std::unique_ptr<ExprAST> ParseUnary() {
  // If the current token is not an operator, it must be a primary expr.
  if (!isascii(CurTok) || CurTok == '(' || CurTok == ',') {
    return ParsePrimary();
  }

  // If this is a unary operator, read it.
  int Opc = CurTok;
  getNextToken();
  if (auto Operand = ParseUnary()) {
    return std::make_unique<UnaryExprAST>(Opc, std::move(Operand));
  }
  return nullptr;
}

/**
 * @brief
 * @param ExprPrec
 * @param LHS
 * @return
 * binoprhs
 *   ::= ('+' unary)*
 */
std::unique_ptr<ExprAST> ParseBinOpRHS(
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
    int BinOp = CurTok;
    SourceLocation BinLoc = CurLoc;
    getNextToken();  // eat binop

    // Parse the unary expression after the binary operator.
    auto RHS = ParseUnary();
    if (!RHS) {
      return nullptr;
    }

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
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
 * @brief
 * @return
 * expression
 *   ::= unary binoprhs
 *
 */
std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParseUnary();
  if (!LHS) {
    return nullptr;
  }

  return ParseBinOpRHS(0, std::move(LHS));
}

/**
 * @brief
 * @return
 * prototype
 *   ::= id '(' id* ')'
 *   ::= binary LETTER number? (id, id)
 *   ::= unary LETTER (id)
 */
std::unique_ptr<PrototypeAST> ParsePrototype() {
  std::string FnName;

  SourceLocation FnLoc = CurLoc;

  unsigned Kind = 0;  // 0 = identifier, 1 = unary, 2 = binary.
  unsigned BinaryPrecedence = 30;

  switch (CurTok) {
    default:
      return LogError<PrototypeAST>("Expected function name in prototype");
    case tok_identifier:
      FnName = IdentifierStr;
      Kind = 0;
      getNextToken();
      break;
    case tok_unary:
      getNextToken();
      if (!isascii(CurTok)) {
        return LogError<PrototypeAST>("Expected unary operator");
      }
      FnName = "unary";
      FnName += (char) CurTok;
      Kind = 1;
      getNextToken();
      break;
    case tok_binary:
      getNextToken();
      if (!isascii(CurTok)) {
        return LogError<PrototypeAST>("Expected binary operator");
      }
      FnName = "binary";
      FnName += (char) CurTok;
      Kind = 2;
      getNextToken();

      /** Read the precedence if present. */
      if (CurTok == tok_number) {
        if (NumVal < 1 || NumVal > 100) {
          return LogError<PrototypeAST>("Invalid precedence: must be 1..100");
        }
        BinaryPrecedence = (unsigned) NumVal;
        getNextToken();
      }
      break;
  }

  if (CurTok != '(') {
    return LogError<PrototypeAST>("Expected '(' in the function definition.");
  }

  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier) {
    ArgNames.push_back(IdentifierStr);
  }
  if (CurTok != ')') {
    return LogError<PrototypeAST>("Expected ')' in the function definition.");
  }

  // success. //
  getNextToken();  // eat ')'.

  if (CurTok != ':') {
    return LogError<PrototypeAST>("Expected ':' in the function definition");
  }

  getNextToken();  // eat ':'.

  // Verify right number of names for operator. //
  if (Kind && ArgNames.size() != Kind) {
    return LogError<PrototypeAST>("Invalid number of operands for operator");
  }

  return std::make_unique<PrototypeAST>(
    FnLoc, FnName, ArgNames, Kind != 0, BinaryPrecedence);
}

/**
 * @brief
 * @return
 * definition ::= 'function' prototype expression
 */
std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken();  // eat function.
  auto Proto = ParsePrototype();
  if (!Proto) {
    return nullptr;
  }

  if (auto E = ParseExpression()) {
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief
 * @return
 * toplevelexpr ::= expression
 */
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  SourceLocation FnLoc = CurLoc;
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>(
      FnLoc, "__anon_expr", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/**
 * @brief
 * @return
 * external ::= 'extern' prototype
 */
std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken();  // eat extern.
  return ParsePrototype();
}

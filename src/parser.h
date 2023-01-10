#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <llvm/Support/raw_ostream.h>

#include "lexer.h"
#include "utils.h"

/**
 * @brief Include `llvm/IR/Function.h`
 *
 *
 */
namespace llvm {
  class Function;
}

/**
 * @brief Include `llvm/IR/Value.h`
 *
 *
 */
namespace llvm {
  class Value;
}

enum class ExprKind {
  NumberKind = 1,
  VariableKind = 2,
  UnaryKind = 3,
  BinaryKind = 4,
  CallKind = 5,
  IfKind = 6,
  ForKind = 7,
  VarKind = 8,
  PrototypeKind = 9,
  FunctionKind = 10,
  GenericKind = -1
};

class Visitor;

/**
 * @brief Base class for all expression nodes.
 *
 *
 */
class ExprAST {
 public:
  ExprKind kind;
  SourceLocation Loc;

  /**
   * @param Loc
   */
  ExprAST(SourceLocation Loc = Lexer::CurLoc) : Loc(Loc) {
    this->kind = ExprKind::GenericKind;
  }

  virtual ~ExprAST() = default;

  int getLine() const {
    return Loc.Line;
  }

  int getCol() const {
    return Loc.Col;
  }

  void accept(Visitor* visitor);
};

/**
 * @brief Expression class for numeric literals like "1.0".
 *
 *
 */
class NumberExprAST : public ExprAST {
 public:
  /**
   *
   * @return
   */
  double Val;

  NumberExprAST(double Val) : Val(Val) {
    this->kind = ExprKind::NumberKind;
  }
};

/**
 * @brief Expression class for referencing a variable, like "a".
 *
 */
class VariableExprAST : public ExprAST {
 public:
  std::string Name;

  /**
   * @param Loc
   * @param Name
   */
  VariableExprAST(SourceLocation Loc, std::string Name)
      : ExprAST(Loc), Name(std::move(Name)) {
    this->kind = ExprKind::VariableKind;
  }

  const std::string& getName() const {
    return Name;
  }
};

/**
 * @brief Expression class for a unary operator.
 *
 */
class UnaryExprAST : public ExprAST {
 public:
  char Opcode;
  std::unique_ptr<ExprAST> Operand;

  /**
   * @param Opcode
   * @param Operand
   */
  UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand)
      : Opcode(Opcode), Operand(std::move(Operand)) {
    this->kind = ExprKind::UnaryKind;
  }
};

/**
 * @brief Expression class for a binary operator.
 *
 */
class BinaryExprAST : public ExprAST {
 public:
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

  /**
   * @param Loc
   * @param Op
   * @param LHS
   * @param RHS
   */
  BinaryExprAST(
    SourceLocation Loc,
    char Op,
    std::unique_ptr<ExprAST> LHS,
    std::unique_ptr<ExprAST> RHS)
      : ExprAST(Loc), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
    this->kind = ExprKind::BinaryKind;
  }
};

/**
 * @brief Expression class for function calls.
 *
 */
class CallExprAST : public ExprAST {
 public:
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

  /**
   * @param Loc
   * @param Callee
   * @param Args
   */
  CallExprAST(
    SourceLocation Loc,
    std::string Callee,
    std::vector<std::unique_ptr<ExprAST>> Args)
      : ExprAST(Loc), Callee(std::move(Callee)), Args(std::move(Args)) {
    this->kind = ExprKind::CallKind;
  }
};

/**
 * @brief Expression class for if/then/else.
 *
 */
class IfExprAST : public ExprAST {
 public:
  std::unique_ptr<ExprAST> Cond, Then, Else;

  /**
   * @param Loc
   * @param Cond
   * @param Then
   * @param Else
   */
  IfExprAST(
    SourceLocation Loc,
    std::unique_ptr<ExprAST> Cond,
    std::unique_ptr<ExprAST> Then,
    std::unique_ptr<ExprAST> Else)
      : ExprAST(Loc),
        Cond(std::move(Cond)),
        Then(std::move(Then)),
        Else(std::move(Else)) {
    this->kind = ExprKind::IfKind;
  }
};

/**
 * @brief Expression class for for/in.
 *
 *
 */
class ForExprAST : public ExprAST {
 public:
  std::string VarName;
  std::unique_ptr<ExprAST> Start, End, Step, Body;

  /**
   * @param VarName
   * @param Start
   * @param End
   * @param Step
   * @param Body
   */
  ForExprAST(
    std::string VarName,
    std::unique_ptr<ExprAST> Start,
    std::unique_ptr<ExprAST> End,
    std::unique_ptr<ExprAST> Step,
    std::unique_ptr<ExprAST> Body)
      : VarName(std::move(VarName)),
        Start(std::move(Start)),
        End(std::move(End)),
        Step(std::move(Step)),
        Body(std::move(Body)) {
    this->kind = ExprKind::ForKind;
  }
};

/**
 * @brief Expression class for var/in
 *
 *
 */
class VarExprAST : public ExprAST {
 public:
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
  std::unique_ptr<ExprAST> Body;

  /**
   * @param VarNames
   * @param Body
   */
  VarExprAST(
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames,
    std::unique_ptr<ExprAST> Body)
      : VarNames(std::move(VarNames)), Body(std::move(Body)) {
    this->kind = ExprKind::VarKind;
  }
};

/**
 @brief This class represents the "prototype" for a function.

 Captures a function's name, and its argument names (thus implicitly the number
 of arguments the function takes), as well as if it is an operator.
 */
class PrototypeAST : public ExprAST {
 public:
  std::string Name;
  std::vector<VariableExprAST*> Args;
  int Line;

  /**
   * @param Loc
   * @param Name
   * @param Args
   */
  PrototypeAST(
    SourceLocation Loc, std::string Name, std::vector<VariableExprAST*> Args)
      : Name(std::move(Name)), Args(std::move(Args)), Line(Loc.Line) {
    this->kind = ExprKind::PrototypeKind;
  }

  const std::string& getName() const {
    return Name;
  }

  int getLine() const {
    return Line;
  }
};

/**
 * @brief This class represents a function definition itself.
 *
 *
 */
class FunctionAST : public ExprAST {
 public:
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

  /**
   * @param Proto
   * @param Body
   */
  FunctionAST(
    std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {
    this->kind = ExprKind::FunctionKind;
  }
};

class TreeAST : public ExprAST {
 public:
  std::vector<std::unique_ptr<ExprAST>> nodes;
};

class Visitor {
 public:
  virtual void visit(NumberExprAST*) = 0;
  virtual void visit(VariableExprAST*) = 0;
  virtual void visit(UnaryExprAST*) = 0;
  virtual void visit(BinaryExprAST*) = 0;
  virtual void visit(CallExprAST*) = 0;
  virtual void visit(IfExprAST*) = 0;
  virtual void visit(ForExprAST*) = 0;
  virtual void visit(VarExprAST*) = 0;
  virtual void visit(PrototypeAST*) = 0;
  virtual void visit(FunctionAST*) = 0;
  virtual void clean() = 0;
};

class Parser {
 public:
  static std::map<char, int> BinopPrecedence;

  static void setup() {
    Parser::BinopPrecedence['='] = 2;
    Parser::BinopPrecedence['<'] = 10;
    Parser::BinopPrecedence['+'] = 20;
    Parser::BinopPrecedence['-'] = 20;
    Parser::BinopPrecedence['*'] = 40;
  }

  static auto parse() -> TreeAST*;

  static auto GetTokPrecedence() -> int;

  static std::unique_ptr<FunctionAST> ParseDefinition();
  static std::unique_ptr<PrototypeAST> ParseExtern();
  static std::unique_ptr<FunctionAST> ParseTopLevelExpr();
  static std::unique_ptr<ExprAST> ParsePrimary();
  static std::unique_ptr<ExprAST> ParseExpression();
  static std::unique_ptr<IfExprAST> ParseIfExpr();
  static std::unique_ptr<NumberExprAST> ParseNumberExpr();
  static std::unique_ptr<ExprAST> ParseParenExpr();
  static std::unique_ptr<ExprAST> ParseIdentifierExpr();
  static std::unique_ptr<ForExprAST> ParseForExpr();
  static std::unique_ptr<VarExprAST> ParseVarExpr();
  static std::unique_ptr<ExprAST> ParseUnary();
  static std::unique_ptr<ExprAST> ParseBinOpRHS(
    int ExprPrec, std::unique_ptr<ExprAST> LHS);
  static std::unique_ptr<PrototypeAST> ParsePrototype();
  static std::unique_ptr<PrototypeAST> ParseExternPrototype();
};

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

extern SourceLocation CurLoc;

enum class ExprKind {
  NumberKind,
  VariableKind,
  UnaryKind,
  BinaryKind,
  CallKind,
  IfKind,
  ForKind,
  VarKind,
  PrototypeKind,
  FunctionKind
};

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
  ExprAST(SourceLocation Loc = CurLoc) : Loc(Loc) {}

  virtual ~ExprAST() = default;

  int getLine() const {
    return Loc.Line;
  }

  int getCol() const {
    return Loc.Col;
  }

  virtual llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) {
    return out << ':' << getLine() << ':' << getCol() << '\n';
  }
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    return ExprAST::dump(out << Val, ind);
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    return ExprAST::dump(out << Name, ind);
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "unary" << Opcode, ind);
    Operand->dump(out, ind + 1);
    return out;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "binary" << Op, ind);
    LHS->dump(indent(out, ind) << "LHS:", ind + 1);
    RHS->dump(indent(out, ind) << "RHS:", ind + 1);
    return out;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "call " << Callee, ind);
    for (const auto& Arg : Args)
      Arg->dump(indent(out, ind + 1), ind + 1);
    return out;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "if", ind);
    Cond->dump(indent(out, ind) << "Cond:", ind + 1);
    Then->dump(indent(out, ind) << "Then:", ind + 1);
    Else->dump(indent(out, ind) << "Else:", ind + 1);
    return out;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "for", ind);
    Start->dump(indent(out, ind) << "Cond:", ind + 1);
    End->dump(indent(out, ind) << "End:", ind + 1);
    Step->dump(indent(out, ind) << "Step:", ind + 1);
    Body->dump(indent(out, ind) << "Body:", ind + 1);
    return out;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "var", ind);
    for (const auto& NamedVar : VarNames)
      NamedVar.second->dump(
          indent(out, ind) << NamedVar.first << ':', ind + 1);
    Body->dump(indent(out, ind) << "Body:", ind + 1);
    return out;
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
  std::vector<std::string> Args;
  bool IsOperator;
  unsigned Precedence;  // Precedence if a binary op.
  int Line;

  /**
   * @param Loc
   * @param Name
   * @param Args
   * @param IsOperator
   * @param Prec
   */
  PrototypeAST(
      SourceLocation Loc,
      std::string Name,
      std::vector<std::string> Args,
      bool IsOperator = false,
      unsigned Prec = 0)
      : Name(std::move(Name)),
        Args(std::move(Args)),
        IsOperator(IsOperator),
        Precedence(Prec),
        Line(Loc.Line) {
    this->kind = ExprKind::PrototypeKind;
  }

  const std::string& getName() const {
    return Name;
  }

  bool isUnaryOp() const {
    return IsOperator && Args.size() == 1;
  }

  bool isBinaryOp() const {
    return IsOperator && Args.size() == 2;
  }

  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
  }

  unsigned getBinaryPrecedence() const {
    return Precedence;
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

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) {
    indent(out, ind) << "FunctionAST\n";
    ++ind;
    indent(out, ind) << "Body:";
    return Body ? Body->dump(out, ind) : out << "null\n";
  }
};

extern std::map<char, int> BinopPrecedence;

std::unique_ptr<FunctionAST> ParseDefinition();
std::unique_ptr<PrototypeAST> ParseExtern();
std::unique_ptr<FunctionAST> ParseTopLevelExpr();

std::unique_ptr<ExprAST> ParsePrimary();

// declared as extern for testing //
std::unique_ptr<ExprAST> ParseExpression();
std::unique_ptr<IfExprAST> ParseIfExpr();
std::unique_ptr<NumberExprAST> ParseNumberExpr();

/**
 * @brief
 *
 * @tparam T
 * @param ptr
 * @return true
 * @return false
 */
template <typename T>
bool is_expr_ptr(T* ptr);

/**
 * @brief
 *
 * @tparam T
 * @param ptr
 * @return true
 * @return false
 */
template <typename T>
bool is_expr_ptr(std::unique_ptr<T> const& ptr);

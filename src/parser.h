#pragma once

#include <llvm/Support/raw_ostream.h>  // for raw_ostream
#include <map>                         // for map
#include <memory>                      // for unique_ptr
#include <string>                      // for string
#include <utility>                     // for move, pair
#include <vector>                      // for vector
#include "lexer.h"                     // for SourceLocation, Lexer
#include "utils.h"                     // for indent

enum class ExprKind {
  GenericKind = -1,

  // variables
  VariableKind = -10,
  VarKind = -11,  // var keyword for variable declaration

  // operators
  UnaryOpKind = -20,
  BinaryOpKind = -21,

  // functions
  PrototypeKind = -30,
  FunctionKind = -31,
  CallKind = -32,

  // control flow
  IfKind = -40,
  ForKind = -41,

  // data types
  NullDTKind = -100,
  BooleanDTKind = -101,
  Int8DTKind = -102,
  UInt8DTKind = -103,
  Int16DTKind = -104,
  UInt16DTKind = -105,
  Int32DTKind = -106,
  UInt32DTKind = -107,
  Int64DTKind = -108,
  UInt64DTKind = -109,
  FloatDTKind = -110,
  DoubleDTKind = -111,
  BinaryDTKind = -112,
  StringDTKind = -113,
  FixedSizeBinaryDTKind = -114,
  Date32DTKind = -115,
  Date64DTKind = -116,
  TimestampDTKind = -117,
  Time32DTKind = -118,
  Time64DTKind = -119,
  Decimal128DTKind = -120,
  Decimal256DTKind = -121,

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
   * @param Loc The token location
   */
  ExprAST(SourceLocation Loc = Lexer::cur_loc) : Loc(Loc) {
    this->kind = ExprKind::GenericKind;
  }

  virtual ~ExprAST() = default;

  int getLine() const {
    return Loc.line;
  }

  int getCol() const {
    return Loc.Col;
  }

  void accept(Visitor& visitor);

  virtual llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) {
    return out << ':' << this->getLine() << ':' << this->getCol() << "\n";
  }
};

/**
 * @brief Expression class for numeric literals like "1.0".
 *
 *
 */
class FloatExprAST : public ExprAST {
 public:
  double val;

  FloatExprAST(double val) : val(val) {
    this->kind = ExprKind::FloatDTKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    return ExprAST::dump(out << this->val, ind);
  }
};

/**
 * @brief Expression class for referencing a variable, like "a".
 *
 */
class VariableExprAST : public ExprAST {
 public:
  std::string name;

  /**
   * @param Loc The token location
   * @param name The variable name
   */
  VariableExprAST(SourceLocation Loc, std::string name)
      : ExprAST(Loc), name(std::move(name)) {
    this->kind = ExprKind::VariableKind;
  }

  const std::string& getName() const {
    return name;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    return ExprAST::dump(out << name, ind);
  }
};

/**
 * @brief Expression class for a unary operator.
 *
 */
class UnaryExprAST : public ExprAST {
 public:
  char op_code;
  std::unique_ptr<ExprAST> operand;

  /**
   * @param op_code The operator code
   * @param operand The operand expression
   */
  UnaryExprAST(char op_code, std::unique_ptr<ExprAST> operand)
      : op_code(op_code), operand(std::move(operand)) {
    this->kind = ExprKind::UnaryOpKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "unary" << op_code, ind);
    this->operand->dump(out, ind + 1);
    return out;
  }
};

/**
 * @brief Expression class for a binary operator.
 *
 */
class BinaryExprAST : public ExprAST {
 public:
  char op;
  std::unique_ptr<ExprAST> lhs, rhs;

  /**
   * @param Loc The token location
   * @param op The operator
   * @param lhs The left hand side expression
   * @param rhs The right hand side expression
   */
  BinaryExprAST(
    SourceLocation Loc,
    char op,
    std::unique_ptr<ExprAST> lhs,
    std::unique_ptr<ExprAST> rhs)
      : ExprAST(Loc), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {
    this->kind = ExprKind::BinaryOpKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "binary" << op, ind);
    this->lhs->dump(indent(out, ind) << "lhs:", ind + 1);
    this->rhs->dump(indent(out, ind) << "rhs:", ind + 1);
    return out;
  }
};

/**
 * @brief Expression class for function calls.
 *
 */
class CallExprAST : public ExprAST {
 public:
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

  /**
   * @param Loc The token location
   * @param callee The function name
   * @param args The function arguments
   */
  CallExprAST(
    SourceLocation Loc,
    std::string callee,
    std::vector<std::unique_ptr<ExprAST>> args)
      : ExprAST(Loc), callee(std::move(callee)), args(std::move(args)) {
    this->kind = ExprKind::CallKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "call " << callee, ind);
    for (auto node = this->args.begin(); node != this->args.end(); ++node) {
      node->get()->dump(indent(out, ind + 1), ind + 1);
    }
    return out;
  }
};

/**
 * @brief Expression class for if/then/else.
 *
 */
class IfExprAST : public ExprAST {
 public:
  std::unique_ptr<ExprAST> cond, Then, else_;

  /**
   * @param Loc The token location
   * @param cond The conditional expression
   * @param Then The `then` branch expression
   * @param else_ The `else` branch expression
   */
  IfExprAST(
    SourceLocation Loc,
    std::unique_ptr<ExprAST> cond,
    std::unique_ptr<ExprAST> Then,
    std::unique_ptr<ExprAST> else_)
      : ExprAST(Loc),
        cond(std::move(cond)),
        Then(std::move(Then)),
        else_(std::move(else_)) {
    this->kind = ExprKind::IfKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "if", ind);
    this->cond->dump(indent(out, ind) << "cond:", ind + 1);
    this->Then->dump(indent(out, ind) << "Then:", ind + 1);
    this->else_->dump(indent(out, ind) << "else_:", ind + 1);
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
  std::string var_name;
  std::unique_ptr<ExprAST> start, end, step, body;

  /**
   * @param var_name The variable name
   * @param start The `start` parameter for the loop
   * @param end The `end` parameter for the loop
   * @param step The incremental value for the loop
   * @param body The body of the for the loop.
   */
  ForExprAST(
    std::string var_name,
    std::unique_ptr<ExprAST> start,
    std::unique_ptr<ExprAST> end,
    std::unique_ptr<ExprAST> step,
    std::unique_ptr<ExprAST> body)
      : var_name(std::move(var_name)),
        start(std::move(start)),
        end(std::move(end)),
        step(std::move(step)),
        body(std::move(body)) {
    this->kind = ExprKind::ForKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "for", ind);
    this->start->dump(indent(out, ind) << "cond:", ind + 1);
    this->end->dump(indent(out, ind) << "end:", ind + 1);
    this->step->dump(indent(out, ind) << "step:", ind + 1);
    this->body->dump(indent(out, ind) << "body:", ind + 1);
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
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> var_names;
  std::unique_ptr<ExprAST> body;

  /**
   * @param var_names Variable names
   * @param body body of the variables
   */
  VarExprAST(
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> var_names,
    std::unique_ptr<ExprAST> body)
      : var_names(std::move(var_names)), body(std::move(body)) {
    this->kind = ExprKind::VarKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "var", ind);
    for (auto node = this->var_names.begin(); node != this->var_names.end();
         ++node) {
      node->second->dump(indent(out, ind) << node->first << ':', ind + 1);
      this->body->dump(indent(out, ind) << "body:", ind + 1);
      return out;
    }
  }
};

/**
 @brief This class represents the "prototype" for a function.

 Captures a function's name, and its argument names (thus implicitly the
 number of arguments the function takes), as well as if it is an operator.
 */
class PrototypeAST : public ExprAST {
 public:
  std::string name;
  std::vector<std::unique_ptr<VariableExprAST>> args;
  int line;

  /**
   * @param Loc The token location
   * @param name The prototype name
   * @param args The prototype arguments
   */
  PrototypeAST(
    SourceLocation Loc,
    std::string name,
    std::vector<std::unique_ptr<VariableExprAST>>&& args)
      : name(std::move(name)), args(std::move(args)), line(Loc.line) {
    this->kind = ExprKind::PrototypeKind;
  }

  const std::string& getName() const {
    return name;
  }

  int getLine() const {
    return line;
  }
};

/**
 * @brief This class represents a function definition itself.
 *
 *
 */
class FunctionAST : public ExprAST {
 public:
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

  /**
   * @param proto The function prototype
   * @param body The function body
   */
  FunctionAST(
    std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
      : proto(std::move(proto)), body(std::move(body)) {
    this->kind = ExprKind::FunctionKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) {
    indent(out, ind) << "FunctionAST\n";
    ++ind;
    indent(out, ind) << "body:";
    return this->body ? this->body->dump(out, ind) : out << "null\n";
  }
};

class TreeAST : public ExprAST {
 public:
  std::vector<std::unique_ptr<ExprAST>> nodes;
};

class Visitor {
 public:
  virtual void visit(FloatExprAST&) = 0;
  virtual void visit(VariableExprAST&) = 0;
  virtual void visit(UnaryExprAST&) = 0;
  virtual void visit(BinaryExprAST&) = 0;
  virtual void visit(CallExprAST&) = 0;
  virtual void visit(IfExprAST&) = 0;
  virtual void visit(ForExprAST&) = 0;
  virtual void visit(VarExprAST&) = 0;
  virtual void visit(PrototypeAST&) = 0;
  virtual void visit(FunctionAST&) = 0;
  virtual void clean() = 0;
  virtual ~Visitor() = default;
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

  static auto parse() -> std::unique_ptr<TreeAST>;

  static auto get_tok_precedence() -> int;

  static std::unique_ptr<FunctionAST> parse_definition();
  static std::unique_ptr<PrototypeAST> parse_extern();
  static std::unique_ptr<FunctionAST> parse_top_level_expr();
  static std::unique_ptr<ExprAST> parse_primary();
  static std::unique_ptr<ExprAST> parse_expression();
  static std::unique_ptr<IfExprAST> parse_if_expr();
  static std::unique_ptr<FloatExprAST> parse_float_expr();
  static std::unique_ptr<ExprAST> parse_paren_expr();
  static std::unique_ptr<ExprAST> parse_identifier_expr();
  static std::unique_ptr<ForExprAST> parse_for_expr();
  static std::unique_ptr<VarExprAST> parse_var_expr();
  static std::unique_ptr<ExprAST> parse_unary();
  static std::unique_ptr<ExprAST> parse_bin_op_rhs(
    int ExprPrec, std::unique_ptr<ExprAST> lhs);
  static std::unique_ptr<PrototypeAST> parse_prototype();
  static std::unique_ptr<PrototypeAST> parse_extern_prototype();
};

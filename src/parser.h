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
  SourceLocation loc;

  /**
   * @param loc The token location
   */
  ExprAST(SourceLocation _loc = Lexer::cur_loc) : loc(_loc) {
    this->kind = ExprKind::GenericKind;
  }

  virtual ~ExprAST() = default;

  int get_line() const {
    return loc.line;
  }

  int getCol() const {
    return loc.col;
  }

  void accept(Visitor& visitor);

  virtual llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) {
    return out << ':' << this->get_line() << ':' << this->getCol() << "\n";
  }
};

/**
 * @brief Expression class for numeric literals like "1.0".
 *
 *
 */
class FloatExprAST : public ExprAST {
 public:
  float val;

  FloatExprAST(float _val) : val(_val) {
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
  std::string type_name;

  /**
   * @param _loc The token location
   * @param _name The variable name
   */
  VariableExprAST(
    SourceLocation _loc, std::string _name, std::string _type_name)
      : ExprAST(_loc),
        name(std::move(_name)),
        type_name(std::move(_type_name)) {
    this->kind = ExprKind::VariableKind;
  }

  const std::string& get_name() const {
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
   * @param _op_code The operator code
   * @param _operand The operand expression
   */
  UnaryExprAST(char _op_code, std::unique_ptr<ExprAST> _operand)
      : op_code(_op_code), operand(std::move(_operand)) {
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
   * @param _loc The token location
   * @param _op The operator
   * @param _lhs The left hand side expression
   * @param _rhs The right hand side expression
   */
  BinaryExprAST(
    SourceLocation _loc,
    char _op,
    std::unique_ptr<ExprAST> _lhs,
    std::unique_ptr<ExprAST> _rhs)
      : ExprAST(_loc), op(_op), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {
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
   * @param _loc The token location
   * @param _callee The function name
   * @param _args The function arguments
   */
  CallExprAST(
    SourceLocation _loc,
    std::string _callee,
    std::vector<std::unique_ptr<ExprAST>> _args)
      : ExprAST(_loc), callee(std::move(_callee)), args(std::move(_args)) {
    this->kind = ExprKind::CallKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "call " << this->callee, ind);
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
  std::unique_ptr<ExprAST> cond, then, else_;

  /**
   * @param _loc The token location
   * @param _cond The conditional expression
   * @param _then The `then` branch expression
   * @param _else_ The `else` branch expression
   */
  IfExprAST(
    SourceLocation _loc,
    std::unique_ptr<ExprAST> _cond,
    std::unique_ptr<ExprAST> _then,
    std::unique_ptr<ExprAST> _else_)
      : ExprAST(_loc),
        cond(std::move(_cond)),
        then(std::move(_then)),
        else_(std::move(_else_)) {
    this->kind = ExprKind::IfKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "if", ind);
    this->cond->dump(indent(out, ind) << "cond:", ind + 1);
    this->then->dump(indent(out, ind) << "then:", ind + 1);
    this->else_->dump(indent(out, ind) << "else:", ind + 1);
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
   * @param _var_name The variable name
   * @param _start The `start` parameter for the loop
   * @param _end The `end` parameter for the loop
   * @param _step The incremental value for the loop
   * @param _body The body of the for the loop.
   */
  ForExprAST(
    std::string _var_name,
    std::unique_ptr<ExprAST> _start,
    std::unique_ptr<ExprAST> _end,
    std::unique_ptr<ExprAST> _step,
    std::unique_ptr<ExprAST> _body)
      : var_name(std::move(_var_name)),
        start(std::move(_start)),
        end(std::move(_end)),
        step(std::move(_step)),
        body(std::move(_body)) {
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

  // todo: implement type_name

  /**
   * @param var_names Variable names
   * @param body body of the variables
   */
  VarExprAST(
    std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> _var_names,
    std::unique_ptr<ExprAST> _body)
      : var_names(std::move(_var_names)), body(std::move(_body)) {
    this->kind = ExprKind::VarKind;
  }

  llvm::raw_ostream& dump(llvm::raw_ostream& out, int ind) override {
    ExprAST::dump(out << "var", ind);
    for (auto node = this->var_names.begin(); node != this->var_names.end();
         ++node) {
      node->second->dump(indent(out, ind) << node->first << ':', ind + 1);
    }
    this->body->dump(indent(out, ind) << "body:", ind + 1);
    return out;
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
  std::string type_name;
  std::vector<std::unique_ptr<VariableExprAST>> args;
  int line;

  /**
   * @param _loc The token location
   * @param _name The prototype name
   * @param _args The prototype arguments
   */
  PrototypeAST(
    SourceLocation _loc,
    std::string _name,
    std::string _type_name,
    std::vector<std::unique_ptr<VariableExprAST>>&& _args)
      : name(std::move(_name)),
        type_name(std::move(_type_name)),
        args(std::move(_args)),
        line(_loc.line) {
    this->kind = ExprKind::PrototypeKind;
  }

  const std::string& get_name() const {
    return name;
  }

  int get_line() const {
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
   * @param _proto The function prototype
   * @param _body The function body
   */
  FunctionAST(
    std::unique_ptr<PrototypeAST> _proto, std::unique_ptr<ExprAST> _body)
      : proto(std::move(_proto)), body(std::move(_body)) {
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
  static std::map<char, int> bin_op_precedence;

  static void setup() {
    Parser::bin_op_precedence['='] = 2;
    Parser::bin_op_precedence['<'] = 10;
    Parser::bin_op_precedence['+'] = 20;
    Parser::bin_op_precedence['-'] = 20;
    Parser::bin_op_precedence['*'] = 40;
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
    int expr_prec, std::unique_ptr<ExprAST> lhs);
  static std::unique_ptr<PrototypeAST> parse_prototype();
  static std::unique_ptr<PrototypeAST> parse_extern_prototype();
};

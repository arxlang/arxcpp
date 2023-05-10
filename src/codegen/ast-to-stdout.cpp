#include "ast-to-stdout.h"
#include <iostream>  // for operator<<, endl, basic_ostream, cout, ostream
#include <memory>    // for allocator, unique_ptr
#include <string>    // for operator<<, string
#include <utility>   // for pair
#include <vector>    // for vector
#include "parser.h"  // for ExprAST, ForExprAST, IfExprAST, BinaryExprAST

int INDENT_SIZE = 2;

class ASTToOutputVisitor
    : public std::enable_shared_from_this<ASTToOutputVisitor>,
      public Visitor {
 public:
  int indent = 0;
  std::string annotation = "";

  ~ASTToOutputVisitor() = default;

  virtual void visit(FloatExprAST*) override;
  virtual void visit(VariableExprAST*) override;
  virtual void visit(UnaryExprAST*) override;
  virtual void visit(BinaryExprAST*) override;
  virtual void visit(CallExprAST*) override;
  virtual void visit(IfExprAST*) override;
  virtual void visit(ForExprAST*) override;
  virtual void visit(VarExprAST*) override;
  virtual void visit(PrototypeAST*) override;
  virtual void visit(FunctionAST*) override;

  virtual void clean() override {}

  auto indentation() -> std::string {
    std::string _indent(this->indent, ' ');
    return _indent;
  }

  auto set_annotation(std::string annotation) -> void {
    this->annotation = annotation;
  }

  auto get_annotation() -> std::string {
    std::string _anno = this->annotation;
    this->annotation = "";
    return _anno;
  }
};

void ASTToOutputVisitor::visit(FloatExprAST* expr) {
  std::cout << this->indentation() << this->get_annotation() << "(Number "
            << expr->Val << ")";
}

void ASTToOutputVisitor::visit(VariableExprAST* expr) {
  std::cout << this->indentation() << this->get_annotation()
            << "(VariableExprAST " << expr->Name << ")";
}

void ASTToOutputVisitor::visit(UnaryExprAST* expr) {
  std::cout << "(UnaryExprAST"
            << ")" << std::endl;
}

void ASTToOutputVisitor::visit(BinaryExprAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  std::cout << this->indentation() << "BinaryExprAST (" << std::endl;
  this->indent += INDENT_SIZE;

  expr->LHS->accept(shared_this);
  std::cout << ", " << std::endl;

  std::cout << this->indentation() << "(OP " << expr->Op << ")," << std::endl;

  expr->RHS->accept(shared_this);
  std::cout << this->indentation() << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(CallExprAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  // start CallExprAST and open the arguments section
  std::cout << this->indentation() << "CallExprAST " << expr->Callee << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  for (auto node = expr->Args.begin(); node != expr->Args.end(); ++node) {
    node->get()->accept(shared_this);
    std::cout << std::endl;
  }

  // close CallExprAST arguments
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  // close CallExprAST
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(IfExprAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  std::cout << this->indentation() << '(' << std::endl;
  this->indent += INDENT_SIZE;

  // open an IfExprAST branch
  std::cout << this->indentation() << "IfExprAST (" << std::endl;
  this->indent += INDENT_SIZE;
  this->set_annotation("<COND>");

  expr->Cond->accept(shared_this);
  std::cout << ',' << std::endl;
  this->set_annotation("<THEN>");

  expr->Then->accept(shared_this);

  if (expr->Else) {
    std::cout << ',' << std::endl;
    this->set_annotation("<ELSE>");
    expr->Else->accept(shared_this);
    std::cout << std::endl;
  } else {
    std::cout << std::endl;
    std::cout << this->indentation() << "()" << std::endl;
  }

  // close IfExprAST body
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  // close IfExprAST
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(ForExprAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  // TODO: implement it
  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  std::cout << this->indentation() << "ForExprAST (" << std::endl;
  this->indent += INDENT_SIZE;

  // start
  this->set_annotation("<START>");
  expr->Start->accept(shared_this);
  std::cout << ", " << std::endl;

  // end
  this->set_annotation("<END>");
  expr->End->accept(shared_this);
  std::cout << ", " << std::endl;

  // step
  this->set_annotation("<STEP>");
  expr->Step->accept(shared_this);
  std::cout << ", " << std::endl;

  // body
  this->set_annotation("<BODY>");
  expr->Body->accept(shared_this);
  std::cout << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(VarExprAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  // TODO: implement it
  std::cout << "(VarExprAST " << std::endl;
  this->indent += INDENT_SIZE;

  for (auto var_expr = expr->VarNames.begin();
       var_expr != expr->VarNames.end();
       ++var_expr) {
    var_expr->second->accept(shared_this);
    std::cout << "," << std::endl;
  }

  this->indent -= INDENT_SIZE;

  std::cout << ")" << std::endl;
}

void ASTToOutputVisitor::visit(PrototypeAST* expr) {
  // TODO: implement it
  std::cout << "(PrototypeAST " << expr->Name << ")" << std::endl;
}

void ASTToOutputVisitor::visit(FunctionAST* expr) {
  std::shared_ptr<ASTToOutputVisitor> shared_this = this->shared_from_this();

  std::cout << this->indentation() << '(' << std::endl;
  this->indent += INDENT_SIZE;

  // create the function and open the args section
  std::cout << this->indentation() << "Function " << expr->Proto->Name
            << " <ARGS> (" << std::endl;
  this->indent += INDENT_SIZE;

  // std::cout << expr->Proto->Args.front();

  for (auto node : expr->Proto->Args) {
    node->accept(shared_this);
    std::cout << ", " << std::endl;
  }

  // close args section and open body section
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << "), " << std::endl
            << this->indentation() << "<BODY> (" << std::endl;

  this->indent += INDENT_SIZE;
  // TODO: Body should be a vector of unique_ptr<Expr>
  expr->Body->accept(shared_this);

  // close body section
  this->indent -= INDENT_SIZE;
  std::cout << std::endl << this->indentation() << ")" << std::endl;

  // close function section
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;
}

auto print_ast(std::unique_ptr<TreeAST> ast) -> void {
  auto visitor_print =
    std::make_shared<ASTToOutputVisitor>(ASTToOutputVisitor());

  std::cout << "[" << std::endl;
  visitor_print->indent += INDENT_SIZE;

  for (auto& node : ast->nodes) {
    node->accept(visitor_print);
    std::cout << visitor_print->indentation() << "," << std::endl;
  }

  std::cout << "]" << std::endl;
}

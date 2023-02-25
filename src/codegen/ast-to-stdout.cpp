#include "ast-to-stdout.h"
#include <iostream>  // for operator<<, endl, basic_ostream, cout, ostream
#include <memory>    // for allocator, unique_ptr
#include <string>    // for operator<<, string
#include <utility>   // for pair
#include <vector>    // for vector
#include "parser.h"  // for ExprAST, ForExprAST, IfExprAST, BinaryExprAST

int INDENT_SIZE = 2;

class ASTToOutputVisitor : public Visitor {
 public:
  int indent = 0;
  std::string annotation = "";

  ~ASTToOutputVisitor() = default;

  virtual void visit(NumberExprAST*) override;
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

void ASTToOutputVisitor::visit(NumberExprAST* expr) {
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
  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  std::cout << this->indentation() << "BinaryExprAST (" << std::endl;
  this->indent += INDENT_SIZE;

  expr->LHS->accept(this);
  std::cout << ", " << std::endl;

  std::cout << this->indentation() << "(OP " << expr->Op << ")," << std::endl;

  expr->RHS->accept(this);
  std::cout << this->indentation() << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(CallExprAST* expr) {
  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  // start CallExprAST and open the arguments section
  std::cout << this->indentation() << "CallExprAST " << expr->Callee << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  for (auto node = expr->Args.begin(); node != expr->Args.end(); ++node) {
    node->get()->accept(this);
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
  std::cout << this->indentation() << '(' << std::endl;
  this->indent += INDENT_SIZE;

  // open an IfExprAST branch
  std::cout << this->indentation() << "IfExprAST (" << std::endl;
  this->indent += INDENT_SIZE;
  this->set_annotation("<COND>");

  expr->Cond->accept(this);
  std::cout << ',' << std::endl;
  this->set_annotation("<THEN>");

  expr->Then->accept(this);

  if (expr->Else) {
    std::cout << ',' << std::endl;
    this->set_annotation("<ELSE>");
    expr->Else->accept(this);
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
  // TODO: implement it
  std::cout << this->indentation() << this->get_annotation() << '('
            << std::endl;
  this->indent += INDENT_SIZE;

  std::cout << this->indentation() << "ForExprAST (" << std::endl;
  this->indent += INDENT_SIZE;

  // start
  this->set_annotation("<START>");
  expr->Start->accept(this);
  std::cout << ", " << std::endl;

  // end
  this->set_annotation("<END>");
  expr->End->accept(this);
  std::cout << ", " << std::endl;

  // step
  this->set_annotation("<STEP>");
  expr->Step->accept(this);
  std::cout << ", " << std::endl;

  // body
  this->set_annotation("<BODY>");
  expr->Body->accept(this);
  std::cout << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;

  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")";
}

void ASTToOutputVisitor::visit(VarExprAST* expr) {
  // TODO: implement it
  std::cout << "(VarExprAST " << std::endl;
  this->indent += INDENT_SIZE;

  for (auto var_expr = expr->VarNames.begin();
       var_expr != expr->VarNames.end();
       ++var_expr) {
    var_expr->second->accept(this);
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
  std::cout << this->indentation() << '(' << std::endl;
  this->indent += INDENT_SIZE;

  // create the function and open the args section
  std::cout << this->indentation() << "Function " << expr->Proto->Name
            << " <ARGS> (" << std::endl;
  this->indent += INDENT_SIZE;

  // std::cout << expr->Proto->Args.front();

  for (auto node : expr->Proto->Args) {
    node->accept(this);
    std::cout << ", " << std::endl;
  }

  // close args section and open body section
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << "), " << std::endl
            << this->indentation() << "<BODY> (" << std::endl;

  this->indent += INDENT_SIZE;
  // TODO: Body should be a vector of unique_ptr<Expr>
  expr->Body->accept(this);

  // close body section
  this->indent -= INDENT_SIZE;
  std::cout << std::endl << this->indentation() << ")" << std::endl;

  // close function section
  this->indent -= INDENT_SIZE;
  std::cout << this->indentation() << ")" << std::endl;
}

auto print_ast(TreeAST* ast) -> void {
  auto visitor_print = new ASTToOutputVisitor();

  std::cout << "[" << std::endl;
  visitor_print->indent += INDENT_SIZE;

  for (auto node = ast->nodes.begin(); node != ast->nodes.end(); ++node) {
    node->get()->accept(visitor_print);
    std::cout << visitor_print->indentation() << "," << std::endl;
  }

  std::cout << "]" << std::endl;
}

#include "ast-to-output.h"
#include <iostream>
#include "parser.h"

class ASTToOutputVisitor : public Visitor {
 public:
  int indent = 0;

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
  virtual void clean() override{};

  auto indentation() -> std::string {
    std::string _indent(this->indent, ' ');
    return _indent;
  }
};

void ASTToOutputVisitor::visit(NumberExprAST* expr) {
  std::cout << "(Number " << expr->Val << ")" << std::endl;
}

void ASTToOutputVisitor::visit(VariableExprAST* expr) {
  std::cout << "VariableExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(UnaryExprAST* expr) {
  std::cout << "UnaryExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(BinaryExprAST* expr) {
  std::cout << "BinaryExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(CallExprAST* expr) {
  std::cout << "CallExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(IfExprAST* expr) {
  std::cout << "IfExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(ForExprAST* expr) {
  std::cout << "ForExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(VarExprAST* expr) {
  std::cout << "VarExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(PrototypeAST* expr) {
  std::cout << "PrototypeAST" << std::endl;
}

void ASTToOutputVisitor::visit(FunctionAST* expr) {
  std::cout << this->indentation() << "(Function " << expr->Proto->Name << " ("
            << std::endl;
  this->indent += 2;

  // std::cout << expr->Proto->Args.front();

  for (auto node : expr->Proto->Args) {
    expr->accept(this);
    // std::cout << node << std::endl;
  }

  this->indent -= 2;
  std::cout << this->indentation() << ")" << std::endl;
}

auto print_ast(TreeAST* tree_ast) -> void {
  auto visitor_print = new ASTToOutputVisitor();

  std::cout << "[" << std::endl;
  visitor_print->indent += 2;

  for (auto node : tree_ast->nodes) {
    node->accept(visitor_print);
  }

  std::cout << "]" << std::endl;
}

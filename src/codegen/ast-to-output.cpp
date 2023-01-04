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
};

void ASTToOutputVisitor::visit(NumberExprAST*) {
  std::cout << "NumberExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(VariableExprAST*) {
  std::cout << "VariableExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(UnaryExprAST*) {
  std::cout << "UnaryExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(BinaryExprAST*) {
  std::cout << "BinaryExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(CallExprAST*) {
  std::cout << "CallExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(IfExprAST*) {
  std::cout << "IfExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(ForExprAST*) {
  std::cout << "ForExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(VarExprAST*) {
  std::cout << "VarExprAST" << std::endl;
}

void ASTToOutputVisitor::visit(PrototypeAST*) {
  std::cout << "PrototypeAST" << std::endl;
}

void ASTToOutputVisitor::visit(FunctionAST*) {
  std::cout << "FunctionAST" << std::endl;
}

auto print_ast(TreeAST* tree_ast) -> void {
  auto visitor_print = new ASTToOutputVisitor();

  for (auto node : tree_ast->nodes) {
    node->accept(visitor_print);
  }
}

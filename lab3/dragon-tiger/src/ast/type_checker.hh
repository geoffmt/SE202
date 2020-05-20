#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include "nodes.hh"

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor {

public:
  TypeChecker() {}
  virtual void visit(IntegerLiteral &);
  virtual void visit(StringLiteral &);
  virtual void visit(Sequence &);
  virtual void visit(IfThenElse &);
  virtual void visit(Let &);
  virtual void visit(VarDecl &);
  virtual void visit(BinaryOperator &);
  virtual void visit(Identifier &);
  virtual void visit(Assign &);
  virtual void visit(WhileLoop &);
  virtual void visit(ForLoop &);
  virtual void visit(Break &);
  virtual void visit(FunDecl &);
  virtual void visit(FunCall &);
};

} // namespace type_checker
} // namespace ast

#endif // TYPE_CHECKER_HH

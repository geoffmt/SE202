#include "type_checker.hh"
#include "../utils/errors.hh"


namespace ast {
namespace type_checker {

void TypeChecker::visit(IntegerLiteral &literal) {
  literal.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &literal) {
  literal.set_type(t_string);
}

void TypeChecker::visit(Sequence &seq)
{
  std::vector<Expr *> exprs = seq.get_exprs();

  Type type = t_void;

  for (Expr * expr : exprs){
    expr->accept(this*);
    type = expr->get_type(); // same type as their last expression
  }

  seq.set_type(type);
}

void TypeChecker::visit(IfThenElse &ite){
  Expr cond = ite.get_condition();
  Expr then = ite.get_then_part();
  Expr _else = ite.get_else_part();

  cond.accept(*this);

  if (cond.get_type()!= t_int){
    utils::error(ite.loc, "Type for condition must be int.")
  }

  then.accept(*this);
  _else.accept(*this);

  if (then.get_type()!=_else.get_type()){
    utils::error(ite.loc, "Branches type not compatible.");
  }

  ite.set_type(_else.get_type());
}

void TypeChecker::visit(Let &let){

}

void TypeChecker::visit(VarDecl &decl){

}

void TypeChecker::visit(BinaryOperator &op){

}

void TypeChecker::visit(Identifier &id){

}

void TypeChecker::visit(Assign &assign){

}

void TypeChecker::visit(WhileLoop &loop){

}

void TypeChecker::visit(ForLoop &loop){

}

void TypeChecker::visit(Break &b){

}

void TypeChecker::visit(FunDecl &decl){

}

void TypeChecker::visit(FunCall &call){

}


} // namespace type_checker
} // namespace ast

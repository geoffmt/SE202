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

void TypeChecker::visit(Let &let)
{
  std::vector<Decl *> &decls = let.get_decls();
  Sequence &seq = let.get_sequence();
  

  for (Decl *decl : decls)
  {
    decl->accept(this *);
  }

  seq.accept(this*);
  let.set_type(seq.get_type());
}

void TypeChecker::visit(VarDecl &decl){




}

void TypeChecker::visit(BinaryOperator &op){
  Expr left = op.get_left();
  Expr right = op.get_right();
  left.accept(this*);
  right.accept(this*);

  if (left.get_type()!=right.get_type()){
    utils:error(op.loc, "Operands do not have the same type.");
  }

  if (left.get_type() == t_void){
    utils::error(op.loc, "Wrong type for operand.")
  }

  op.set_type(t_int);
}

void TypeChecker::visit(Identifier &id){
  optional<VarDecl &> decl = id.get_decl();
  if(decl){
    id.set_type(decl.value().get_type());
  }
  
}

void TypeChecker::visit(Assign &assign){
  Identifier lhs = assign.get_lhs();
  Identifier rhs = assign.get_rhs();

  lhs.accept(this*);
  rhs.accept(this*);

  if (lhs.get_type()!=rhs.get_type()){
    utils::error(assign.loc, "Wrong type.")
  }

  assign.set_type(t_void);
}

void TypeChecker::visit(WhileLoop &loop){




}

void TypeChecker::visit(ForLoop &loop){




}

void TypeChecker::visit(Break &b){
  b.set_type(t_void);
}

void TypeChecker::visit(FunDecl &decl){




}

void TypeChecker::visit(FunCall &call){




}


} // namespace type_checker
} // namespace ast

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
    expr->accept(*this);
    type = expr->get_type(); // same type as their last expression
  }

  seq.set_type(type);
}

void TypeChecker::visit(IfThenElse &ite){
  Expr &cond = ite.get_condition();
  Expr &then = ite.get_then_part();
  Expr &_else = ite.get_else_part();

  cond.accept(*this);

  if (cond.get_type()!= t_int){
    utils::error(ite.loc, "Type for condition must be int.");
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
    decl->accept(*this);
  }

  seq.accept(*this);
  let.set_type(seq.get_type());
}

void TypeChecker::visit(VarDecl &decl)
{
  Type type = t_undef;
  // case with explicit type in type_name field
  if (decl.type_name)
  {
    std::string type_name = std::string(decl.type_name.value());

    // check if type_n is a know type
    if (type_name == "int")
    {
      type = t_int;
    }
    else if (type_name == "string")
    {
      type = t_string;
    }
    else
    {
      utils::error(decl.loc, "Incompatible type.");
    }
  }

  // case without an explicit type given
  Type type_e = t_undef;
  optional<Expr &> expr = decl.get_expr();
  if (expr)
  {
    expr.value().accept(*this);
    type_e = expr.value().get_type();
    if (type_e == t_void)
    {
      utils::error(decl.loc, "Type t_void not allowed.");
    }
  }

  if (type == t_undef && type_e == t_undef)
  {
    utils::error(decl.loc, "Unknown type for variable");
  }
  if (type != t_undef && type_e != t_undef)
  {
    utils::error(decl.loc, "Two different types for variable.");
  }
  if (type != t_undef && type_e == t_undef)
  {
    decl.set_type(type);
  }
  if (type == t_undef && type_e != t_undef)
  {
    decl.set_type(type_e);
  }
}

void TypeChecker::visit(BinaryOperator &op){
  Expr &left = op.get_left();
  Expr &right = op.get_right();
  left.accept(*this);
  right.accept(*this);

  if (left.get_type()!=right.get_type()){
    utils::error(op.loc, "Operands do not have the same type.");
  }

  if (left.get_type() == t_void){
    utils::error(op.loc, "Wrong type for operand.");
  }

  op.set_type(t_int);
}

void TypeChecker::visit(Identifier &id){
  optional<VarDecl &> decl = id.get_decl();
  if(decl){
    id.set_type(decl.value().get_type());
  }
  else{
    utils::error(id.loc, "No declaration.");
  }
  
}

void TypeChecker::visit(Assign &assign){
  Identifier &lhs = assign.get_lhs();
  Expr &rhs = assign.get_rhs();

  lhs.accept(*this);
  rhs.accept(*this);

  if (lhs.get_type()!=rhs.get_type()){
    utils::error(assign.loc, "Wrong type.");
  }

  assign.set_type(t_void);
}

void TypeChecker::visit(WhileLoop &loop){
  loop.get_condition().accept(*this);
  if (loop.get_condition().get_type() != t_int){
    utils::error(loop.loc, "Type for condition is not valid.");
  }
  
  loop.get_body().accept(*this);
  if (loop.get_body().get_type() != t_void){
    utils::error(loop.loc, "Type for loop body is not valid.");
  }

  loop.set_type(t_void);

}

void TypeChecker::visit(ForLoop &loop){
  loop.get_high().accept(*this);
  if (loop.get_high().get_type() != t_int){
    utils::error(loop.loc, "Type for bounds is not valid.");
  }
  
  loop.get_variable().accept(*this);
  if (loop.get_variable().get_type() != t_int){
    utils::error(loop.loc, "Type for variable is not valid.");
  }

  loop.get_body().accept(*this);
  if (loop.get_body().get_type() != t_void){
    utils::error(loop.loc, "Type for loop body is not valid.");
  }

  loop.set_type(t_void);

}

void TypeChecker::visit(Break &b){
  b.set_type(t_void);
}

void TypeChecker::visit(FunDecl &decl){

  std::vector<VarDecl *> &params = decl.get_params();

  // accept parameter of function
  for (VarDecl *decl : params)
  {
    decl->accept(*this);
  }

  Type type = t_undef;

  if (decl.type_name){
    std::string type_name = std::string(decl.type_name.value());

    if (type_name == "int"){
      type = t_int;
    }
    else if (type_name == "string"){
      type = t_string;
    }
    else if (type_name == "void"){
      type = t_void;
    }
    else
    {
      utils::error(decl.loc, "Incompatible type.");
    }
  }

  decl.set_type(type);

  Type type_e = t_undef;
  // accept expr of function
  optional<Expr &> expr = decl.get_expr();
  if (expr)
  {
    expr.value().accept(*this);
    type_e = expr.value().get_type();
    if (type != type_e)
    {
      utils::error(decl.loc, "Two different types for variable.");
    }
  }
  

}

void TypeChecker::visit(FunCall &call){

  optional<FunDecl &> decl = call.get_decl();

  decl.value().accept(*this);
  call.set_type(decl.value().get_type());

  std::vector<VarDecl *> &params = decl.value().get_params();
  std::vector<Expr *> &args = call.get_args();
  
  // check if there is the right number of arguments
  if (args.size() != params.size()){
    utils::error(call.loc, "Number of arguments do not match.");
  }

  // check if they have all the right type
  for (int i = 0; i<args.size(); i++){
    args[i]->accept(*this);
    if (args[i]->get_type() != params[i]->get_type()){
      utils::error(call.loc, "Arguments type do not match.");
    }
  }
}


} // namespace type_checker
} // namespace ast

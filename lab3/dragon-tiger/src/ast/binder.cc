#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  const Symbol s_int = Symbol("int");
  const Symbol s_string = Symbol("string");
  enter_primitive("print_err", boost::none, {s_string});
  enter_primitive("print", boost::none, {s_string});
  enter_primitive("print_int", boost::none, {s_int});
  enter_primitive("flush", boost::none, {});
  enter_primitive("getchar", s_string, {});
  enter_primitive("ord", s_int, {s_string});
  enter_primitive("chr", s_string, {s_int});
  enter_primitive("size", s_int, {s_string});
  enter_primitive("substring", s_string, {s_string, s_int, s_int});
  enter_primitive("concat", s_string, {s_string, s_string});
  enter_primitive("strcmp", s_int, {s_string, s_string});
  enter_primitive("streq", s_int, {s_string, s_string});
  enter_primitive("not", s_int, {s_int});
  enter_primitive("exit", boost::none, {s_int});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const boost::optional<Symbol> &type_name,
    const std::vector<Symbol> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const Symbol &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), nullptr, tn));
  }

  boost::optional<Symbol> type_name_symbol = boost::none;
  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), std::move(args), nullptr,
                            type_name, true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), main_params,
                                    main_body, Symbol("int"), true);
  main->accept(*this);
  return main;
}

void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op)
{
  op.get_left().accept(*this);
  op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq)
{

  std::vector<Expr *> exprs = seq.get_exprs();

  for (Expr *expr : exprs)
  {
    expr->accept(*this);
  }
}

void Binder::visit(Let &let)
{
  push_scope();

  std::vector<Decl *> &decls = let.get_decls();
  Sequence &seq = let.get_sequence();
  std::vector<FunDecl *> funDecls;
  Loop * ex_current_loop = curr_loop;
  curr_loop = nullptr;

  for (auto it = decls.begin(); it != decls.end(); it++)
  {
    FunDecl *decl = dynamic_cast<FunDecl *>(*it);
    if (!decl)
    {
      (*it)->accept(*this);
    }
    // while fundecl we keep consecutive functions
    else
    {
      while (decl)
      {
        enter(*decl);
        funDecls.push_back(decl);
        it++;
        if (it == decls.end())
        {
          break;
        }
        decl = dynamic_cast<FunDecl *>(*it);
      }
      //consecutive func
      for (FunDecl *decl : funDecls)
      {
        decl->accept(*this);
      }
    }
  }

  curr_loop = ex_current_loop;
  seq.accept(*this);
  pop_scope();
}

void Binder::visit(Identifier &id)
{
  VarDecl *decl = dynamic_cast<VarDecl *>(&find(id.loc, id.name)); // Finds the declaration

  // An identifier is used but not declared
  if (!decl)
  {
    error(id.loc, "Identifier declaration not found for " + std::string(id.name));
  }
  id.set_decl(decl);
  id.set_depth(functions.size());

  // is it a escaping ?
  if (decl->get_depth() < id.get_depth()) {
    decl->set_escapes();
  }
}

void Binder::visit(IfThenElse &ite)
{
  ite.get_condition().accept(*this);
  ite.get_then_part().accept(*this);
  ite.get_else_part().accept(*this);
}

void Binder::visit(VarDecl &decl)
{

  optional<Expr &> expr = decl.get_expr(); // optional because get_expr() is declared this way
  if (expr)
  {
    expr.value().accept(*this);
  }
  enter(decl);
  decl.set_depth(functions.size());
}

void Binder::visit(FunDecl &decl)
{
  set_parent_and_external_name(decl);
  functions.push_back(&decl);
  decl.set_depth(functions.size()-1);

  push_scope(); // we go in
  std::vector<VarDecl *> &params = decl.get_params();

  // accept parameter of function
  for (VarDecl *decl : params)
  {
    decl->accept(*this);
  }

  // accept expr of function
  optional<Expr &> expr = decl.get_expr();
  if (expr)
  {
    expr.value().accept(*this);
  }

  pop_scope(); // we go out
  functions.pop_back();
}

void Binder::visit(FunCall &call)
{
  FunDecl *decl = dynamic_cast<FunDecl *>(&find(call.loc, call.func_name));
  if (!decl)
  {
    error(call.loc, "Function declaration not found for " + std::string(call.func_name));
  }

  call.set_decl(decl);
  call.set_depth(functions.size());

  // attempt to solve mutually recursive functions issue
  std::vector<Expr *> &args = call.get_args();
  for (Expr *expr : args)
  {
    expr->accept(*this);
  }
}

void Binder::visit(WhileLoop &loop)
{
  loop.get_condition().accept(*this);

  Loop *ex_current_loop = curr_loop; // we save the previous loop
  curr_loop = &loop;
  loop.get_body().accept(*this);

  curr_loop = ex_current_loop; // to go out of the loop when finished
}

void Binder::visit(ForLoop &loop)
{
  loop.get_high().accept(*this);

  push_scope();
  loop.get_variable().accept(*this);

  Loop *ex_current_loop = curr_loop;
  curr_loop = &loop;
  loop.get_body().accept(*this);
  pop_scope();

  curr_loop = ex_current_loop;
}

void Binder::visit(Break &b)
{
  if (curr_loop)
  {
    b.set_loop(curr_loop);
  }
  else
  {
    error(b.loc, "Break used outside of a loop");
  }
}

void Binder::visit(Assign &assign)
{
  assign.get_lhs().accept(*this);

  assign.get_rhs().accept(*this);
}

} // namespace binder
} // namespace ast

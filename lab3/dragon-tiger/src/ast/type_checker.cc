#include "type_checker.hh"
#include "../utils/errors.hh"

using utils::error;

namespace ast
{
  namespace type_checker
  {

    void TypeChecker::visit(IntegerLiteral &literal)
    {
      literal.set_type(t_int);
    }

    void TypeChecker::visit(StringLiteral &literal)
    {
      literal.set_type(t_string);
    }

    void TypeChecker::visit(Sequence &seq)
    {
      std::vector<Expr *> exprs = seq.get_exprs();

      Type type = t_void;

      for (Expr *expr : exprs)
      {
        expr->accept(*this);
        type = expr->get_type(); // same type as their last expression
      }

      seq.set_type(type);
    }

    void TypeChecker::visit(IfThenElse &ite)
    {
      Expr &cond = ite.get_condition();
      Expr &then = ite.get_then_part();
      Expr &_else = ite.get_else_part();

      cond.accept(*this);

      if (cond.get_type() != t_int)
      {
        error(ite.loc, "Type for condition must be int.");
      }

      then.accept(*this);
      _else.accept(*this);

      if (then.get_type() != _else.get_type())
      {
        error(ite.loc, "Branches type not compatible.");
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
          error(decl.loc, "Incompatible type.");
        }
      }

      // case without an explicit type given
      Type type_e = t_void;
      optional<Expr &> expr = decl.get_expr();
      if (!expr)
      {
        utils::error(decl.loc, "Error: implicit type must have declaration.") ;
      }

      expr.value().accept(*this);
      type_e = expr.value().get_type();

      if (type == t_undef && type_e == t_undef)
      {
        error(decl.loc, "Unknown type for variable");
      }
      
      if (type != t_undef && type_e != t_undef)
      {
        if (type == type_e)
        {
          decl.set_type(type);
        }
        else
        {
          error(decl.loc, "Two different types for variable.");
        }
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

    void TypeChecker::visit(BinaryOperator &op)
    {
      Expr &left = op.get_left();
      Expr &right = op.get_right();
      left.accept(*this);
      right.accept(*this);

      if (left.get_type() == t_int && right.get_type() == t_int)
      {
        op.set_type(t_int);
      }

      else if (left.get_type() == t_string && right.get_type() == t_string)
      {
        if ((op.op == o_plus) || (op.op == o_minus) || (op.op == o_times) || (op.op == o_divide))
          error(op.loc, "Wrong type for operand with this arithmetic expression.");
        op.set_type(t_int);
      }
      else if (left.get_type() == t_void && right.get_type() == t_void)
      {
        if (op.op != o_eq)
        {
          utils::error(op.loc, "Error: Only void variables can be tested for equality.");
        }
        op.set_type(t_int);
      }
      else
      {
        utils::error(op.loc, "Operands do not have the same type.");
      }
    }

    void TypeChecker::visit(Identifier &id)
    {
      optional<VarDecl &> decl = id.get_decl();
      if (decl)
      {
        id.set_type(decl.value().get_type());
      }
      else
      {
        error(id.loc, "No declaration.");
      }
    }

    void TypeChecker::visit(Assign &assign)
    {
      Identifier &lhs = assign.get_lhs();
      Expr &rhs = assign.get_rhs();

      lhs.accept(*this);
      rhs.accept(*this);

      if (lhs.get_type() != rhs.get_type())
      {
        error(assign.loc, "Wrong type.");
      }

      assign.set_type(t_void);
    }

    void TypeChecker::visit(WhileLoop &loop)
    {
      loop.get_condition().accept(*this);
      if (loop.get_condition().get_type() != t_int)
      {
        error(loop.loc, "Type for condition is not valid.");
      }

      loop.get_body().accept(*this);
      if (loop.get_body().get_type() != t_void)
      {
        error(loop.loc, "Type for loop body is not valid.");
      }

      loop.set_type(t_void);
    }

    void TypeChecker::visit(ForLoop &loop)
    {
      loop.get_high().accept(*this);
      if (loop.get_high().get_type() != t_int)
      {
        error(loop.loc, "Type for bounds is not valid.");
      }

      loop.get_variable().accept(*this);
      if (loop.get_variable().get_type() != t_int)
      {
        error(loop.loc, "Type for variable is not valid.");
      }

      loop.get_body().accept(*this);
      if (loop.get_body().get_type() != t_void)
      {
        error(loop.loc, "Type for loop body is not valid.");
      }

      loop.set_type(t_void);
    }

    void TypeChecker::visit(Break &b)
    {
      b.set_type(t_void);
    }

    void TypeChecker::visit(FunDecl &decl)
    {
      if (decl.get_type() == t_undef)
      {
        std::vector<VarDecl *> &params = decl.get_params();

        // accept parameter of function
        for (VarDecl *decl : params)
        {
          decl->accept(*this);
        }

        Type type = t_void;

        if (decl.type_name)
        {
          std::string type_name = std::string(decl.type_name.value());

          if (type_name == "int")
          {
            type = t_int;
          }
          else if (type_name == "string")
          {
            type = t_string;
          }
          else if (type_name == "void")
          {
            // is it primitive
            if (decl.is_external)
            {
              type = t_void;
            }
            else
            {
              error(decl.loc, "Explicit void type name is not allowed in non-primitive function declaration.");
            }
          }
          else
          {
            error(decl.loc, "Incompatible type.");
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
            error(decl.loc, "Two different types for variable.");
          }
        }
      }
    }

    void TypeChecker::visit(FunCall &call)
    {

      optional<FunDecl &> decl = call.get_decl();
      std::vector<VarDecl *> &params = decl.value().get_params();
      std::vector<Expr *> &args = call.get_args();

      // check if there is the right number of arguments
      if (args.size() != params.size())
      {
        error(call.loc, "Number of arguments do not match.");
      }

      if (decl.value().get_type() == t_undef)
      {
        decl.value().accept(*this);
      }
      call.set_type(decl.value().get_type());

      // check if they have all the right type
      for (long unsigned int i = 0; i < args.size(); i++)
      {
        args[i]->accept(*this);
        if (args[i]->get_type() != params[i]->get_type())
        {
          error(call.loc, "Arguments type do not match.");
        }
      }
    }

  } // namespace type_checker
} // namespace ast

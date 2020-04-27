#ifndef AST_EVALUATOR_HH
#define AST_EVALUATOR_HH

#include "nodes.hh"

namespace ast
{

class ASTEvaluator : public ConstASTIntVisitor
{
public:
	ASTEvaluator() {}

	virtual int visit(const IntegerLiteral &);
	virtual int visit(const StringLiteral &);
	virtual int visit(const BinaryOperator &);
	virtual int visit(const Sequence &);
	virtual int visit(const Let &);
	virtual int visit(const Identifier &);
	virtual int visit(const IfThenElse &);
	virtual int visit(const VarDecl &);
	virtual int visit(const FunDecl &);
	virtual int visit(const FunCall &);
	virtual int visit(const WhileLoop &);
	virtual int visit(const ForLoop &);
	virtual int visit(const Break &);
	virtual int visit(const Assign &);
};

} // namespace ast

#endif
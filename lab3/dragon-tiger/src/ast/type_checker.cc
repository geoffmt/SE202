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

} // namespace type_checker
} // namespace ast

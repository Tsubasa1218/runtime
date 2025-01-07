#include <vector>

#include "types.hpp"

namespace React {

struct Expression {};

struct Constant : Expression {
  Answer value;

  explicit Constant(Answer value) : Expression(), value(value) {}
};

struct FormElementReference : Expression {
  BuilderId id;

  explicit FormElementReference(BuilderId id) : Expression(), id(id) {}
};

struct IfThenElse : Expression {
  Expression predicate;
  Expression consequence;
  Expression alternative;

  explicit IfThenElse(Expression p, Expression c, Expression a)
      : Expression(), predicate(p), consequence(c), alternative(a) {}
};

enum BinaryOperationType {
  AND,
  OR,
  EQUALS,
  NOT_EQUALS,
};

struct BinaryOperation : Expression {
  BinaryOperationType op;
  Expression lhs;
  Expression rhs;

  explicit BinaryOperation(BinaryOperationType op, Expression lhs,
                           Expression rhs)
      : Expression(), op(op), lhs(lhs), rhs(rhs) {}
};

enum UnaryOperationType {
  IS_ANSWERED,
  IS_NOT_ANSWERED,
};

struct UnaryOperation : Expression {
  UnaryOperationType op;
  Expression operand;

  explicit UnaryOperation(UnaryOperationType op, Expression operand)
      : Expression(), op(op), operand(operand) {}
};

struct Block : Expression {
  std::vector<Expression> expressions;

  explicit Block(std::vector<Expression> es) : Expression(), expressions(es) {}
};

struct VarAssignment : Expression {
  BuilderId target_id;
  Expression value;

  explicit VarAssignment(BuilderId target_id, Expression value)
      : Expression(), target_id(target_id), value(value) {}
};

// Constant
// Question Reference
// IF <predicate> THEN <consequence> ELSE <alternative>
// predicate := <binary op> | <unary op>
// binary expr := <expr> <op> <expr>
// unary expr := <op> <expr>
// assignment
// function calling
} // namespace React

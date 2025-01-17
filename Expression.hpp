#include <atomic>
#include <memory>
#include <variant>

#include "Reactive/Reactive.hpp"
#include "types.hpp"

template <typename T> struct Constant {
  T value;
};

template <typename T> struct FormElementReference {
  std::shared_ptr<Signal<T>> question_signal;
};

template <typename T>
using FormElementOrConstantReference =
    std::variant<Constant<T>, FormElementReference<T>>;

template <typename T> struct VarAssignment {
  FormElementReference<T> target_question;
  FormElementOrConstantReference<T> value;
};

enum ConditionOperator {
  EQUALS,
};

template <typename T> struct Condition {
  const BuilderId condition_id;
  ConditionOperator op;
  FormElementReference<T> lhs;
  FormElementOrConstantReference<T> rhs;
};

enum RuleLinkType { JOIN, CONDITION };

struct RuleLink {
  RuleLinkType type;
  BuilderId target_id;
};

enum JoinOp { AND, OR };

struct RuleJoin {
  BuilderId join_id;
  JoinOp op;
  RuleLink lhs, rhs;
};

struct IfThenElse {
  RuleJoin predicate;
};

// Constant
// Question Reference
// IF <predicate> THEN <consequence> ELSE <alternative>
// predicate := <binary op> | <unary op>
// binary expr := <expr> <op> <expr>
// unary expr := <op> <expr>
// assignment
// function calling
//

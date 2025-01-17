#include <algorithm>
#include <ios>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "Expression.hpp"
#include "FormElements.hpp"
#include "Reactive/Reactive.hpp"

namespace ranges = std::ranges;

using Conditions = std::variant<Condition<float>, Condition<bool>,
                                Condition<int>, Condition<std::string>>;
using ConditionsVec = std::vector<Conditions>;
using JoinVec = std::vector<RuleJoin>;
using JoinExprVec = std::vector<RuleLink>;

struct CLContext {
  ConditionsVec const &all_conditions;
  JoinVec const &all_joins;
  JoinExprVec const &all_join_expr;
};

template <typename T> bool eval_condition(Condition<T> const &condition) {
  T lhs_value = condition.lhs.question_signal->get();

  if (auto *constant = std::get_if<Constant<T>>(&condition.rhs)) {
    return lhs_value == constant->value;
  }

  if (auto *rhs_question =
          std::get_if<FormElementReference<T>>(&condition.rhs)) {
    return lhs_value == rhs_question->question_signal->get();
  }

  return false;
}

template <class... Ts> struct ConditionVisitors : Ts... {
  using Ts::operator()...;
};

template <typename T>
void check_both_condition_arms(Condition<T> const &c, RuleJoin const &join,
                               bool &out_lhs, bool &out_rhs) {
  if (c.condition_id == join.lhs.target_id) {
    out_lhs = eval_condition<T>(c);
  } else if (c.condition_id == join.rhs.target_id) {
    out_rhs = eval_condition<T>(c);
  }
}

void eval_join_expr(RuleLink const &join_expr,
                    ConditionsVec const &all_conditions,
                    JoinVec const &all_joins) {

};

template <typename T>
bool is_target_condition(Condition<T> const &c, RuleLink const &link) {
  return c.condition_id == link.target_id;
}

bool eval_join(RuleJoin const &join, CLContext const &context) {
  auto [all_conditions, all_joins, all_join_expr] = context;

  bool lhs_value = false;
  bool rhs_value = false;

  if (join.lhs.type == RuleLinkType::CONDITION &&
      join.rhs.type == RuleLinkType::CONDITION) {

    for (auto const &c : all_conditions) {
      std::visit(
          [&](auto c) {
            return check_both_condition_arms(c, join, lhs_value, rhs_value);
          },
          c);
    }
  }

  if (join.lhs.type == RuleLinkType::CONDITION &&
      join.rhs.type == RuleLinkType::JOIN) {

    for (auto const &c : all_conditions) {
      std::visit(
          [&](auto c) {
            if (is_target_condition(c, join.lhs)) {
              lhs_value = eval_condition(c);
            }
          },
          c);
    }

    if (auto target_join = ranges::find_if(
            all_joins,
            [&](RuleJoin j) { return j.join_id == join.rhs.target_id; });
        target_join != all_joins.end()) {
      rhs_value = eval_join(*target_join, context);
    }
  }

  if (join.lhs.type == RuleLinkType::JOIN &&
      join.rhs.type == RuleLinkType::CONDITION) {

    for (auto const &c : all_conditions) {
      std::visit(
          [&](auto c) {
            if (is_target_condition(c, join.rhs)) {
              lhs_value = eval_condition(c);
            }
          },
          c);
    }

    if (auto target_join = ranges::find_if(
            all_joins,
            [&](RuleJoin j) { return j.join_id == join.lhs.target_id; });
        target_join != all_joins.end()) {
      rhs_value = eval_join(*target_join, context);
    }
  }

  if (join.lhs.type == RuleLinkType::JOIN &&
      join.rhs.type == RuleLinkType::JOIN) {
    for (auto const &j : all_joins) {
      if (j.join_id == join.lhs.target_id) {
        lhs_value = eval_join(j, context);
        continue;
      }

      if (j.join_id == join.rhs.target_id) {
        rhs_value = eval_join(j, context);
      }
    }
  }

  if (join.op == JoinOp::AND) {
    return lhs_value && rhs_value;
  }
  if (join.op == JoinOp::OR) {
    return lhs_value && rhs_value;
  }

  return false;
}

void eval_if_statement(IfThenElse const &if_statement,
                       CLContext const &context) {
  if (eval_join(if_statement.predicate, context)) {
    std::cout << "Predicate is true\n";
  } else {
    std::cout << "Predicate is false\n";
  }
}

int main() {
  std::cout << std::boolalpha;

  Runtime runtime{};

  auto s_1 = std::make_shared<Signal<float>>(runtime.create_signal(1.2f));
  auto s_2 = std::make_shared<Signal<float>>(runtime.create_signal(1.2f));
  auto s_3 = std::make_shared<Signal<bool>>(runtime.create_signal(false));
  auto s_4 = std::make_shared<Signal<int>>(runtime.create_signal(42));

  auto question_1 =
      Question<float>(1, "Q1", "Question 1", true, false, false, s_1);

  auto question_2 =
      Question<float>(2, "Q2", "Question 2", true, false, false, s_2);

  auto question_3 =
      Question<bool>(3, "Q3", "Question 3", true, false, false, s_3);

  auto question_4 =
      Question<int>(4, "Q4", "Question 4", true, false, false, s_4);

  auto q1_equals_q2 = Condition<float>{1, ConditionOperator::EQUALS,
                                       FormElementReference{question_1.value},
                                       FormElementReference{question_2.value}};

  auto q3_equals_true = Condition<bool>{2, ConditionOperator::EQUALS,
                                        FormElementReference{question_3.value},
                                        Constant<bool>{true}};

  auto q4_equals_69 =
      Condition<int>{3, ConditionOperator::EQUALS,
                     FormElementReference{question_4.value}, Constant<int>{69}};

  ConditionsVec conditions{q1_equals_q2, q3_equals_true, q4_equals_69};

  JoinExprVec join_exprs = {};

  JoinVec joins = {};

  RuleLink join_1{RuleLinkType::CONDITION, q1_equals_q2.condition_id};
  RuleLink join_2{RuleLinkType::CONDITION, q3_equals_true.condition_id};
  RuleJoin q1_equals_q2_AND_q3_equals_true{1, JoinOp::AND, join_1, join_2};
  join_exprs.emplace_back(join_1);
  join_exprs.emplace_back(join_2);
  joins.emplace_back(q1_equals_q2_AND_q3_equals_true);

  RuleLink join_3{RuleLinkType::JOIN, q1_equals_q2_AND_q3_equals_true.join_id};
  RuleLink join_4{RuleLinkType::CONDITION, q4_equals_69.condition_id};
  RuleJoin other_or_q3_equals_69{2, JoinOp::OR, join_3, join_4};
  join_exprs.emplace_back(join_3);
  join_exprs.emplace_back(join_4);
  joins.emplace_back(other_or_q3_equals_69);

  // (Q1 == Q2 AND Q3 == True) || Q3 == 69
  IfThenElse rule{other_or_q3_equals_69};

  CLContext context{conditions, joins, join_exprs};

  runtime.create_effect([&] {
    eval_if_statement(rule, context);
    std::cout << "S1 = " << s_1->get() << "\n";
    std::cout << "S2 = " << s_2->get() << "\n";
    std::cout << "S3 = " << s_3->get() << "\n";
    std::cout << "S4 = " << s_4->get() << "\n\n";
  });

  s_3->set(true);
  s_4->set(69);

  /*

    auto is_lhs_float_question = [&](QuestionPtr<float> q) {
      return q->builder_id == c.lhs.question_id;
    };

    if (auto lhs = ranges::find_if(float_questions, is_lhs_float_question);
        lhs != float_questions.end()) {

      std::optional<float> rhs_value = [&] {
        if (const auto *rhs = std::get_if<FormElementReference>(&c.rhs)) {
          auto rhs_question =
              ranges::find_if(float_questions, [&](QuestionPtr<float> o) {
                return o->builder_id == rhs->question_id;
              });

          if (rhs_question == float_questions.end()) {
            return std::nullopt;
          }

          return std::make_optional(rhs_question->value.get());
        }

        if (const auto *rhs = std::get_if<ConstantReference>(&c.rhs)) {
          auto rhs_question = ranges::find_if(float_questions, [&](std::any o) {
              try {
                auto v = std::any_cast<Constant<float>>(o);
                return v.constant_id == rhs->constant_id;
              } catch {

              }
          });
        }
      }();
    }
  }*/

  /*

    conditions.emplace_back(q3_is_true);

    Join and_expr{3, JoinOp::AND,
                      JoinExpr{JoinExprType::CONDITION, 1},
                      JoinExpr{JoinExprType::CONDITION, 2}};

    ActionBlock consequence{};
    consequence.add_action(VarAssignment<std::string>(
        question_4.value, Constant{std::string("Answered")}));

    ActionBlock alternative{};
    alternative.add_action(VarAssignment<std::string>(
        question_4.value, Constant<std::string>("Not Answered!")));

    IfThenElse if_statement(and_expr, consequence, alternative);
    */
}

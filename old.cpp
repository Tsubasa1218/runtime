#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

typedef unsigned int BuilderId;

typedef std::variant<int, float, std::string> QuestionAnswer;

struct Question {
  const BuilderId builder_id;
  bool required;
  bool read_only;
  bool visible;
  std::optional<QuestionAnswer> value;
};

struct Section {
  const BuilderId builder_id;
  bool visible;
};

struct Page {
  const BuilderId builder_id;
  bool visible;
};

enum ConditionOp {
  EQUALS,
  NOT_EQUALS,
  IS_ANSWERED,
  IS_NOT_ANSWERED,
};

struct RuleCondition {
  const BuilderId builder_id;
  const ConditionOp op;
  Question const &lhs;
  std::optional<QuestionAnswer> target_value;
  std::optional<std::shared_ptr<Question>> target_question;

  const bool evaluate() {
    switch (this->op) {
    case ConditionOp::EQUALS: {
      if (!this->lhs.value.has_value())
        return false;

      const QuestionAnswer lhs_value = this->lhs.value.value();

      if (this->target_value.has_value()) {
        return lhs_value == this->target_value;
      }

      if (this->target_question.has_value()) {
        return lhs_value == target_question.value()->value;
      }

      return false;
    };

    case ConditionOp::NOT_EQUALS: {
      if (!this->lhs.value.has_value())
        return false;

      const QuestionAnswer lhs_value = this->lhs.value.value();

      if (this->target_value.has_value()) {
        return lhs_value != this->target_value;
      }

      if (this->target_question.has_value()) {
        return lhs_value != target_question.value()->value;
      }

      return false;
    };

    case ConditionOp::IS_ANSWERED:
      return this->lhs.value.has_value();

    case ConditionOp::IS_NOT_ANSWERED:
      return !this->lhs.value.has_value();
    default:
      return false;
    }
  }
};

std::function<bool(bool, bool)> AND_OP = [](bool lhs, bool rhs) {
  return lhs && rhs;
};

std::function<bool(bool, bool)> OR_OP = [](bool lhs, bool rhs) {
  return lhs || rhs;
};

struct RuleJoin {
  const BuilderId builder_id;
  std::function<bool(bool, bool)> op;
  std::variant<struct RuleJoin *, RuleCondition> lhs;
  std::variant<struct RuleJoin *, RuleCondition> rhs;

  const bool evaluate() {
    bool lhs_result = false;
    bool rhs_result = false;

    if (auto *left_condition = std::get_if<RuleCondition>(&this->lhs)) {
      lhs_result = left_condition->evaluate();
    }

    if (auto *right_condition = std::get_if<RuleCondition>(&this->rhs)) {
      rhs_result = right_condition->evaluate();
    }

    if (auto **left_rule = std::get_if<struct RuleJoin *>(&this->lhs)) {
      auto *temp = *left_rule;
      lhs_result = temp->evaluate();
    }

    if (auto **right_rule = std::get_if<struct RuleJoin *>(&this->rhs)) {
      auto *temp = *right_rule;
      rhs_result = temp->evaluate();
    }

    return this->op(lhs_result, rhs_result);
  }
};

enum ActionOp {
  SET_VALUE,
  CLEAR_VALUE,

  SET_REQUIRED,
  SET_NOT_REQUIRED,

  SET_READ_ONLY,
  SET_NOT_READ_ONLY,

  SET_VISIBLE,
  SET_HIDDEN,
};

struct Action {
  const ActionOp op;
  Question const &target;
  std::optional<std::variant<const QuestionAnswer, std::shared_ptr<Question>>>
      target_value;
};

struct LogicElement {
  std::vector<RuleJoin> rules;
  std::vector<Action> actions;
};

const Question q1{1, true, true, true, 123};
const Question q2{2, true, true, true, "Hello, world!"};
const Question q3{3, false, false, false, 2.51f};

const RuleCondition is_answered_condition{
    .builder_id = 5,
    .op = ConditionOp::IS_ANSWERED,
    .lhs = q1,
    .target_value = std::nullopt,
};

const RuleCondition is_run_condition{
    .builder_id = 6,
    .op = ConditionOp::EQUALS,
    .lhs = q2,
    .target_value = "RUN",
};

const std::vector<RuleJoin> rules{{
    .builder_id = 4,
    .op = AND_OP,
    .lhs = is_answered_condition,
    .rhs = is_run_condition,
}};

const std::vector<Action> actions{{
    .op = ActionOp::SET_VALUE,
    .target = q3,
    .target_value = 420.69f,
}};

// IF A is answered AND B == "RUN" THEN C is set to 420.69
LogicElement le1{rules, actions};

for (auto &r : le1.rules) {
  std::cout << r.builder_id << " = " << r.evaluate() << std::endl;
}

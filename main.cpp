#include <string>
#include <vector>

#include "Expression.hpp"
#include "Reactive/Reactive.hpp"
#include "types.hpp"

template <typename T> struct Question {
  const BuilderId builder_id;
  const std::string label, question_text;
  bool visible, read_only, required;
  Signal<T> value;
};

struct Section {
  const BuilderId builder_id;
  const std::string name, label;
  bool visible;
  std::vector<BuilderId> questions;
};

struct Page {
  const BuilderId builder_id;
  const std::string name, label;
  bool visible;
  std::vector<BuilderId> sections;
};

int main() {
  Runtime runtime{};

  Question<float> question_1{
      1, "Q1", "Question 1", true, false, false, runtime.create_signal(1.2f)};

  Question<float> question_2{
      2, "Q2", "Question 2", true, false, false, runtime.create_signal(1.f)};

  Question<bool> question_3{
      3, "Q3", "Question 3", true, false, false, runtime.create_signal(false)};

  Question<std::string> question_4{4,
                                   "Q4",
                                   "Question 4",
                                   true,
                                   false,
                                   false,
                                   runtime.create_signal(std::string(""))};

  React::FormElementReference q1_ref{question_1.builder_id};
  React::FormElementReference q2_ref{question_2.builder_id};
  React::FormElementReference q3_ref{question_3.builder_id};

  React::BinaryOperation EQUALS{React::BinaryOperationType::EQUALS, q1_ref,
                                q2_ref};
  React::BinaryOperation AND{React::BinaryOperationType::AND, EQUALS, q3_ref};

  React::Block consequence{{React::VarAssignment{
      question_4.builder_id, React::Constant{"Answered!"}}}};

  React::Block alternative{{React::VarAssignment{
      question_4.builder_id, React::Constant{"Not Answered :("}}}};

  React::IfThenElse if_statement{
      AND,
      consequence,
      alternative,
  };
}

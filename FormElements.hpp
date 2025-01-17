#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Reactive/Reactive.hpp"
#include "types.hpp"

template <typename T> struct Question {
  const BuilderId builder_id;
  const std::string label, question_text;
  bool visible, read_only, required;
  std::shared_ptr<Signal<T>> value;
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

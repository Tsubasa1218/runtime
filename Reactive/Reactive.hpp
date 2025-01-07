#pragma once
#include <any>
#include <cstddef>
#include <functional>
#include <optional>
#include <set>
#include <vector>

using SignalId = const size_t;
using EffectId = size_t;

template <typename T> class Signal;

struct Runtime {
  std::vector<std::any> signal_values;
  std::optional<EffectId> running_effect_id;
  std::vector<std::function<void(void)>> effects;

  Runtime() : signal_values({}), running_effect_id(std::nullopt), effects({}) {}

  template <typename T> Signal<T> create_signal(T initial_value) {
    signal_values.emplace_back(initial_value);
    SignalId new_signal_id = signal_values.size() - 1;

    return Signal<T>(*this, new_signal_id);
  }

  void create_effect(std::function<void(void)> effect) {
    effects.emplace_back(effect);

    EffectId effect_id = effects.size() - 1;

    run_effect(effect_id);
  }

  void run_effect(EffectId effect_id) {
    auto prev_running_effect = running_effect_id;

    running_effect_id = std::make_optional(effect_id);
    effects.at(effect_id)();

    running_effect_id = prev_running_effect;
  }
};

template <typename T> class Signal {
  Runtime &runtime;
  SignalId signal_id;
  std::set<EffectId> signal_subs;

public:
  Signal(Runtime &runtime, SignalId id) : runtime(runtime), signal_id(id) {}

  const T get() {
    T value = std::any_cast<T>(runtime.signal_values.at(signal_id));

    if (runtime.running_effect_id) {
      signal_subs.emplace(runtime.running_effect_id.value());
    }

    return value;
  }

  void set(T new_value) {
    runtime.signal_values.at(signal_id) = new_value;

    for (const auto sub_id : signal_subs) {
      runtime.run_effect(sub_id);
    }
  }
};


#include <string>
#include <variant>

using BuilderId = unsigned int;

using Answer = std::variant<std::string, float, int, bool>;

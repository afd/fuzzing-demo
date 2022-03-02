#include <functional>
#include <iostream>
#include <string>

#include "src/solver.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " input string" << std::endl;
    return 1;
  }
  std::function<void(const std::string &)> error_handler =
      [](const std::string &message) { std::cerr << message << std::endl; };
  std::string input(argv[1]);
  auto instance = Parse(input, error_handler);
  if (!instance.has_value()) {
    return 1;
  }
  auto result = Solve(instance.value());
  if (!result.has_value()) {
    std::cout << "UNSAT" << std::endl;
  } else {
    std::cout << "SAT:";
    for (auto literal : result.value()) {
      std::cout << " " << literal;
    }
    std::cout << std::endl;
  }
  return 0;
}

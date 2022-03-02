#include <functional>
#include <iostream>

#include "src/solver.h"

std::string GetToken(const std::string &input, size_t &index) {
  while (true) {
    char current = input[index];
    if (current != ' ' && current != '\n') {
      break;
    }
    index++;
  }
  std::string result;
  while (true) {
    // BUG: 'index' can be out of bounds
    char current = input[index];
    index++;
    if (current == ' ' || current == '\n') {
      break;
    }
    result += current;
  }
  return result;
}

std::optional<SatInstance> BCP(const SatInstance &instance,
                               std::unordered_set<int32_t> &assignment,
                               std::unordered_set<int32_t> &unassigned) {
  std::optional<SatInstance> result = instance;
  while (true) {
    std::optional<int32_t> unit = {};
    for (auto &clause : result.value().clauses) {
      if (clause.size() == 1) {
        unit = {*clause.begin()};
        break;
      }
    }
    if (!unit.has_value()) {
      return result;
    }
    assert(assignment.find(unit.value()) == assignment.end());
    assert(assignment.find(-unit.value()) == assignment.end());
    assignment.insert(unit.value());
    int32_t variable = unit.value() > 0 ? unit.value() : -unit.value();
    assert(unassigned.count(variable) == 1);
    unassigned.erase(variable);
    result = result.value().Assert(unit.value());
    if (!result.has_value()) {
      return {};
    }
  }
}

std::optional<std::unordered_set<int32_t>>
SolveRecursive(SatInstance instance, std::unordered_set<int32_t> assignment,
               std::unordered_set<int32_t> unassigned) {
  auto simplified_instance = BCP(instance, assignment, unassigned);
  if (!simplified_instance.has_value()) {
    return {};
  }
  if (unassigned.empty()) {
    return {assignment};
  }
  int32_t decision_var = *unassigned.begin();

  {
    unassigned.erase(decision_var);
    assignment.insert(decision_var);
    auto maybe_result =
        SolveRecursive(simplified_instance.value().Assert(decision_var).value(),
                       assignment, unassigned);
    if (maybe_result.has_value()) {
      return maybe_result;
    }
    assignment.erase(decision_var);
  }
  {
    assignment.insert(-decision_var);
    return SolveRecursive(
        simplified_instance.value().Assert(-decision_var).value(), assignment,
        unassigned);
  }
}

std::optional<std::unordered_set<int32_t>> Solve(const SatInstance &instance) {
  std::unordered_set<int32_t> assignment;
  std::unordered_set<int32_t> unassigned;
  for (size_t i = 1; i <= instance.num_vars; i++) {
    unassigned.insert(static_cast<int32_t>(i));
  }
  return SolveRecursive(instance, assignment, unassigned);
}

std::optional<std::unordered_set<int32_t>>
SolveNaiveRecursive(SatInstance instance,
                    std::unordered_set<int32_t> assignment,
                    std::unordered_set<int32_t> unassigned) {
  if (unassigned.empty()) {
    return {assignment};
  }
  int32_t decision_var = *unassigned.begin();

  {
    unassigned.erase(decision_var);
    assignment.insert(decision_var);
    auto maybe_result = SolveNaiveRecursive(
        instance.Assert(decision_var).value(), assignment, unassigned);
    if (maybe_result.has_value()) {
      return maybe_result;
    }
    assignment.erase(decision_var);
  }
  {
    // Bug 1: should be -decision_var
    // Bug 2: should call SolveNaiveRecursive
    assignment.insert(decision_var);
    return SolveRecursive(instance.Assert(-decision_var).value(), assignment,
                          unassigned);
  }
}

std::optional<std::unordered_set<int32_t>>
SolveNaive(const SatInstance &instance) {
  std::unordered_set<int32_t> assignment;
  std::unordered_set<int32_t> unassigned;
  for (size_t i = 1; i <= instance.num_vars; i++) {
    unassigned.insert(static_cast<int32_t>(i));
  }
  return SolveNaiveRecursive(instance, assignment, unassigned);
}

bool IsSatisfyingAssignment(const SatInstance &instance,
                            const std::unordered_set<int32_t> &assignment) {
  std::optional<SatInstance> remaining{instance};
  for (auto literal : assignment) {
    remaining = remaining.value().Assert(literal);
    if (!remaining.has_value()) {
      return false;
    }
  }
  return true;
}

std::optional<SatInstance>
Parse(const std::string &input,
      const std::function<void(const std::string &)> &error_consumer) {
  size_t index = 0;
  if (GetToken(input, index) != "p") {
    error_consumer("Expected p as first token of DIMACS CNF formula");
    return {};
  }
  if (GetToken(input, index) != "cnf") {
    error_consumer("Expected cnf as second token of DIMACS CNF formula");
    return {};
  }
  size_t num_vars = atoi(GetToken(input, index).c_str());
  size_t num_clauses = atoi(GetToken(input, index).c_str());
  std::vector<std::unordered_set<int32_t>> clauses;
  for (size_t i = 0; i < num_clauses; i++) {
    std::unordered_set<int32_t> clause;
    while (true) {
      int32_t literal = atoi(GetToken(input, index).c_str());
      if (literal == 0) {
        break;
      }
      clause.insert(literal);
    }
    clauses.push_back(clause);
  }
  return {{num_vars, clauses}};
}

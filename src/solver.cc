#include <functional>
#include <iostream>
#include <sstream>

#include "src/solver.h"

std::optional<std::string> GetToken(const std::string &input, size_t &index) {
  if (index >= input.size()) {
    return {};
  }
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
    if (index == input.size()) {
      break;
    }
    char current = input[index];
    index++;
    if (current == ' ' || current == '\n') {
      break;
    }
    result += current;
  }
  if (result.empty()) {
    return {};
  }
  return {result};
}

std::optional<SatInstance> BCP(const SatInstance &instance,
                               std::unordered_set<int32_t> &assignment,
                               std::unordered_set<int32_t> &unassigned) {
  std::optional<SatInstance> result = instance;
  while (true) {
    std::optional<int32_t> unit = {};
    int num_large_clauses = 0;
    for (auto &clause : result.value().clauses) {
      if (clause.size() > 8) {
        num_large_clauses++;
        assert(num_large_clauses < 5);
      }
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
  // BUG (performance): we should cease solving if there are no more clauses
  // to satisfy
  // if (simplified_instance->clauses.empty()) {
  //  for (auto var : unassigned) {
  //    assignment.insert(var);
  //  }
  //  return {assignment};
  //}
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
    // BUG: should be -decision_var
    // BUG: should call SolveNaiveRecursive
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
    // BUG (injected)
    if (literal == 10) {
      return false;
    }
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
  // BUG: try-catch required because std::stroi may throw an exception.
  try {
  size_t index = 0;
  auto tok = GetToken(input, index);
  if (!tok.has_value()) {
    error_consumer("Unexpected end of input");
    return {};
  }
  if (tok != "p") {
    error_consumer("Expected p as first token of DIMACS CNF formula");
    return {};
  }
  tok = GetToken(input, index);
  if (!tok.has_value()) {
    error_consumer("Unexpected end of input");
    return {};
  }
  if (tok != "cnf") {
    error_consumer("Expected cnf as second token of DIMACS CNF formula");
    return {};
  }
  tok = GetToken(input, index);
  if (!tok.has_value()) {
    error_consumer("Unexpected end of input");
    return {};
  }
  // BUG: a negative number will lead to a HUGE number of variables
  size_t num_vars = std::stoi(tok.value());
  tok = GetToken(input, index);
  if (!tok.has_value()) {
    error_consumer("Unexpected end of input");
    return {};
  }
  size_t num_clauses = std::stoi(tok.value());
  std::vector<std::unordered_set<int32_t>> clauses;
  for (size_t i = 0; i < num_clauses; i++) {
    std::unordered_set<int32_t> clause;
    while (true) {
      tok = GetToken(input, index);
      // BUG: need to check whether token exists
      if (!tok.has_value()) {
        error_consumer("Unexpected end of input");
        return {};
      }
      int32_t literal = std::stoi(tok.value());
      if (literal == 0) {
        break;
      }
      // BUG: we only want literals that are in range
      // if (literal > static_cast<int>(num_vars) ||
      //    literal < -static_cast<int>(num_vars)) {
      //  std::stringstream stringstream;
      //  stringstream << "Literal " << literal << " out of range";
      //  error_consumer(stringstream.str());
      //  return {};
      //}
      clause.insert(literal);
    }
    // BUG: empty clauses are not allowed
    // if (clause.empty()) {
    //  error_consumer("Empty clauses are not allowed");
    //  return {};
    //}
    clauses.push_back(clause);
  }
  // BUG: nicer if we check there was not excess input
  // tok = GetToken(input, index);
  // if (tok.has_value()) {
  //  error_consumer("Too much input");
  //  return {};
  //}
  return {{num_vars, clauses}};
  } catch (const std::invalid_argument&) {
    error_consumer("Parse error");
    return {};
  } catch (const std::out_of_range&) {
    error_consumer("Parse error");
    return {};
  }
}

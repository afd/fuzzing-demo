#ifndef SOLVER_H
#define SOLVER_H

#include <cassert>
#include <cinttypes>
#include <functional>
#include <optional>
#include <unordered_set>
#include <vector>

struct SatInstance {
  size_t num_vars;
  std::vector<std::unordered_set<int32_t>> clauses;

  std::optional<SatInstance> Assert(int32_t literal) {
    SatInstance result{num_vars, {}};
    for (auto &clause : clauses) {
      assert(clause.size() > 0);
      if (clause.find(literal) != clause.end()) {
        continue;
      }
      std::unordered_set<int32_t> new_clause;
      for (int32_t clause_literal : clause) {
        assert(clause_literal != literal);
        if (clause_literal != -literal) {
          new_clause.insert(clause_literal);
        }
      }
      if (new_clause.empty()) {
        return {};
      }
      result.clauses.push_back(new_clause);
    }
    return {result};
  }
};

std::optional<SatInstance>
Parse(const std::string &input,
      const std::function<void(const std::string &)> &error_consumer);

std::optional<std::unordered_set<int32_t>> Solve(const SatInstance &instance);

std::optional<std::unordered_set<int32_t>>
SolveNaive(const SatInstance &instance);

bool IsSatisfyingAssignment(const SatInstance &instance,
                            const std::unordered_set<int32_t> &assignment);

#endif // SOLVER_H
